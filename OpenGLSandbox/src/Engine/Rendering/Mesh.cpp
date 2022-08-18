#include "epch.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Core/Math.h"
#include "Engine/Rendering/Texture.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#define LOG_MESH 1

namespace Engine
{
	static constexpr uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_JoinIdenticalVertices |
		aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
		aiProcess_ValidateDataStructure;    // Validation


#define PI 3.14159265359

	struct LogStream : Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		void write(const char* message) override
		{
			LOG_ERROR("Assimp Error: {}", message);
		}
	};

	Mesh::Mesh()
	{

	}
	
	Mesh::Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
	{
		m_Vertices.resize(vertexCount);
		memcpy(m_Vertices.data(), vertices, vertexCount * sizeof(Vertex));

		m_Indices.resize(indexCount);
		memcpy(m_Indices.data(), indices, indexCount * sizeof(uint32_t));
	}
	
	Mesh::Mesh(const std::string& modelFilePath)
	{
		LogStream::Initialize();
		LOG_INFO("Loading Mesh: {}", modelFilePath);
		m_Importer = CreateRef<Assimp::Importer>();
		m_Importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		if (const aiScene* pScene = m_Importer->ReadFile(modelFilePath.c_str(), s_MeshImportFlags))
			InitializeMeshFromFile(pScene, modelFilePath);
		else
			LOG_ERROR("Failed to parse model file: {}", modelFilePath);
	}

	static std::tuple<std::string, std::string> TexPathName(const std::string& modelFilePath, const std::string& aiTexPath)
	{
		std::filesystem::path path = modelFilePath;
		const std::string lastForwardSlash = modelFilePath.substr(0, modelFilePath.find_last_of("/") + 1);
		size_t pos = aiTexPath.find_last_of("\\") + 1;
		std::string texName = aiTexPath.substr(pos, aiTexPath.length() - pos);
		std::string texPath = lastForwardSlash + texName;
		return std::make_tuple(texPath, texName);
	}

	static void LogTextureSamplerSet(TextureUniform& Uniform, const Ref<Texture2D>& WhiteTexture)
	{
		LOG_TRACE("\tSetting Sampler Uniform: ");
		LOG_TRACE("\t\tSamplerName: sampler_AlbedoTexture");
		LOG_TRACE("\t\tTextureName: '{}'", WhiteTexture->GetName());
		LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
	}

	void Mesh::InitializeMeshFromFile(const aiScene* scene, const std::string& modelFilePath)
	{
		m_Scene = scene;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		m_MeshShader = ShaderLibrary::Get("EnginePBR");


		for (unsigned m = 0; m < scene->mNumMeshes; m++)
		{
			const aiMesh* mesh = scene->mMeshes[m];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.VertexCount = mesh->mNumVertices;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			for (size_t i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
				vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
				if (mesh->HasTangentsAndBitangents())
				{
					vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
					vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
				}

				if (mesh->HasTextureCoords(0))
					vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

				m_Vertices.push_back(vertex);
			}

			// Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				m_Indices.push_back(mesh->mFaces[i].mIndices[0]);
				m_Indices.push_back(mesh->mFaces[i].mIndices[1]);
				m_Indices.push_back(mesh->mFaces[i].mIndices[2]);
			}
		}
		TraverseNodes(scene->mRootNode);

		Texture2DSpecification Specification
		{
			Engine::ImageUtils::WrapMode::Repeat,
			Engine::ImageUtils::WrapMode::Repeat,
			Engine::ImageUtils::FilterMode::LinearMipLinear,
			Engine::ImageUtils::FilterMode::LinearMipLinear,
			Engine::ImageUtils::ImageInternalFormat::FromImage,
			Engine::ImageUtils::ImageDataLayout::FromImage,
			Engine::ImageUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> whiteTexture = TextureLibrary::Get2D("White Texture");
		int32_t WhiteTextureUnit = 0;
		if (scene->HasMaterials())
		{
			m_Textures.resize(scene->mNumMaterials);
			m_Materials.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{
				int32_t TexUnitCounter = 1;
				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();
				std::cout << aiMaterialName.data << " (Index = " << i << ")" << "\n";
				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
#if LOG_MESH
				LOG_TRACE("\tTextureCount = {}", textureCount);
#endif
				std::string MaterialName = std::string(aiMaterial->GetName().data) + " Material";
#if LOG_MESH
				LOG_TRACE("\tGenerating Material: '{}'.  Material #: {}", MaterialName, i); 
#endif
				auto mi = CreateRef<Material>(MaterialName, m_MeshShader);
				m_Materials[i] = mi;

				glm::vec3 albedoColor(0.8f);
				float emission = 0.0f;
				aiColor3D aiColor, aiEmission;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS)
					albedoColor = { aiColor.r, aiColor.g, aiColor.b };

				if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmission) == AI_SUCCESS)
					emission = aiEmission.r;

				mi->Set<glm::vec3>("AlbedoColor", albedoColor);
				mi->Set<float>("Emission", emission);

				float shininess, metalness;
				if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
					shininess = 80.0f; // Default value

				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
					metalness = 0.0f;

				float roughness = 1.0f - glm::sqrt(shininess / 100.0f);
				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				bool fallback = !hasAlbedoMap;
				if (hasAlbedoMap)
				{
					if(auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						Specification.Width = aiTexEmbedded->mWidth;
						Specification.Height = aiTexEmbedded->mHeight;
						Specification.InternalFormat = ImageUtils::ImageInternalFormat::RGB8;
						Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RGB;
						Specification.DataType = ImageUtils::ImageDataType::UByte;
						Ref<Texture2D> texture = TextureLibrary::LoadTexture2D(Specification, aiTexEmbedded->pcData);
						m_Textures[i] = texture;
						TextureUniform Uniform {texture->GetID(), TexUnitCounter++};
						mi->Set<TextureUniform>("sampler_AlbedoTexture", Uniform);
#if LOG_MESH
						LOG_TRACE("\tSetting Embedded -> Sampler Uniform: ");
						LOG_TRACE("\t\tSamplerName: sampler_AlbedoTexture");
						LOG_TRACE("\t\tTextureName: '{}'", texture->GetName());
						LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
						mi->Set<glm::vec3>("AlbedoColor", glm::vec3(1.0f));
					}
					else
					{
#if LOG_MESH

						LOG_TRACE("\tCreating Albedo Map Texture...");
#endif
						auto [texPath, texName] = TexPathName(modelFilePath, std::string(aiTexPath.data));
						Specification.Name = texName;
						if (auto texture = TextureLibrary::LoadTexture2D(texPath))
						{
							m_Textures[i] = texture;
							TextureUniform Uniform {texture->GetID(), TexUnitCounter++};
							mi->Set<TextureUniform>("sampler_AlbedoTexture", Uniform);
#if LOG_MESH
							LOG_TRACE("\tSetting Sampler Uniform: ");
							LOG_TRACE("\t\tSamplerName: sampler_AlbedoTexture");
							LOG_TRACE("\t\tTextureName: '{}'", texture->GetName());
							LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif

							mi->Set<glm::vec3>("AlbedoColor", glm::vec3(1.0f));
						}
						else
						{
							m_Textures[i] = whiteTexture;
							fallback = true;
#if LOG_MESH
							LOG_WARN("\tCould not load texture: '{}'", texPath);
#endif
						}
					}
				}

				if (fallback)
				{
					TextureUniform Uniform {whiteTexture->GetID(), WhiteTextureUnit};
					mi->Set<TextureUniform>("sampler_AlbedoTexture", Uniform);

					LOG_TRACE("\tSetting Sampler Uniform: ");
					LOG_TRACE("\t\tSamplerName: sampler_AlbedoTexture");
					LOG_TRACE("\t\tTextureName: '{}'", whiteTexture->GetName());
					LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
				}

				// Normal maps
				bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasNormalMap;
				if (hasNormalMap)
				{
					std::cout << "	Creating Normal Map..." << "\n";
					auto [texturePath, texName] = TexPathName(modelFilePath, std::string(aiTexPath.data));
					Specification.Name = texName;
					if (auto texture = TextureLibrary::LoadTexture2D(texturePath))
					{
						m_Textures.push_back(texture);
						TextureUniform Uniform {texture->GetID(), TexUnitCounter++};
						mi->Set<TextureUniform>("sampler_NormalTexture", Uniform);

#if LOG_MESH
						LOG_TRACE("\tSetting Sampler Uniform: ");
						LOG_TRACE("\t\tSamplerName: sampler_NormalTexture");
						LOG_TRACE("\t\tTextureName: '{}'", texture->GetName());
						LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
						mi->Set<int>("UseNormalMap", 1);
					}
					else
					{
						fallback = true;
						LOG_WARN("\tCould not load texture: '{}'", texturePath);
					}
				}

				if (fallback)
				{
					TextureUniform Uniform {whiteTexture->GetID(), WhiteTextureUnit};
					mi->Set<TextureUniform>("sampler_NormalTexture", Uniform);
#if LOG_MESH
					LOG_TRACE("\tSetting Sampler Uniform: ");
					LOG_TRACE("\t\tSamplerName: sampler_NormalTexture");
					LOG_TRACE("\t\tTextureName: '{}'", whiteTexture->GetName());
					LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
					mi->Set<int>("UseNormalMap", 0);
				}

				// Roughness map
				bool hasRoughnessMap = aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasRoughnessMap;
				if (hasRoughnessMap)
				{
					std::cout << "	Creating Roughness Map..." << "\n";
					auto [texturePath, texName] = TexPathName(modelFilePath, std::string(aiTexPath.data));
					Specification.Name = texName;
					if (auto texture = TextureLibrary::LoadTexture2D(texturePath))
					{
						m_Textures.push_back(texture);
						TextureUniform Uniform {texture->GetID(), TexUnitCounter++};
						mi->Set<TextureUniform>("sampler_RoughnessTexture", Uniform);

#if LOG_MESH
						LOG_TRACE("\tSetting Sampler Uniform: ");
						LOG_TRACE("\t\tSamplerName: sampler_RoughnessTexture");
						LOG_TRACE("\t\tTextureName: '{}'", texture->GetName());
						LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
						mi->Set<float>("Roughness", 1.0f);
					}
					else
					{
						fallback = true;
#if LOG_MESH
						LOG_WARN("\tCould not load texture: '{}'", texturePath);
#endif
					}
				}

				if (fallback)
				{
					TextureUniform Uniform {whiteTexture->GetID(), WhiteTextureUnit};
					mi->Set<TextureUniform>("sampler_RoughnessTexture", Uniform);
#if LOG_MESH
					LOG_TRACE("\tSetting Sampler Uniform: ");
					LOG_TRACE("\t\tSamplerName: sampler_RoughnessTexture");
					LOG_TRACE("\t\tTextureName: '{}'", whiteTexture->GetName());
					LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
					mi->Set<float>("Roughness", roughness);
				}

				bool metalnessTextureFound = false;
				for (uint32_t p = 0; p < aiMaterial->mNumProperties; p++)
				{
					auto prop = aiMaterial->mProperties[p];

					if (prop->mType == aiPTI_String)
					{
						uint32_t strLength = *(uint32_t*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file")
						{
							std::cout << "	Creating Metalness Map..." << "\n";
							auto [texturePath, texName] = TexPathName(modelFilePath, std::string(aiTexPath.data));
							Specification.Name = texName;
							if (auto texture = TextureLibrary::LoadTexture2D(texturePath))
							{
								metalnessTextureFound = true;
								m_Textures.push_back(texture);
								TextureUniform Uniform {texture->GetID(), TexUnitCounter++};
								mi->Set<TextureUniform>("sampler_MetalnessTexture", Uniform);

#if LOG_MESH
								LOG_TRACE("\tSetting Sampler Uniform: ");
								LOG_TRACE("\t\tSamplerName: sampler_MetalnessTexture");
								LOG_TRACE("\t\tTextureName: '{}'", texture->GetName());
								LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif

								mi->Set<float>("Metalness", 1.0f);
							}
							else
							{
#if LOG_MESH
								LOG_WARN("\tCould not load texture: '{}'", texturePath);
#endif
							}
							break;
						}
					}
				}

				fallback = !metalnessTextureFound;
				if (fallback)
				{
					TextureUniform Uniform {whiteTexture->GetID(), WhiteTextureUnit};
					mi->Set<TextureUniform>("sampler_MetalnessTexture", Uniform);
#if LOG_MESH
					LOG_TRACE("\tSetting Sampler Uniform: ");
					LOG_TRACE("\t\tSamplerName: sampler_MetalnessTexture");
					LOG_TRACE("\t\tTextureName: '{}'", whiteTexture->GetName());
					LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
#endif
					mi->Set<float>("Metalness", 1.0f);
				}
			}
		}
		else
		{
			for(uint32_t i = 0; i < m_Submeshes.size(); i++)
			{
				TextureUniform Uniform {whiteTexture->GetID(), WhiteTextureUnit};
				auto mi = CreateRef<Material>("Engine-Default", m_MeshShader);

				mi->Set<glm::vec3>("AlbedoColor", glm::vec3(0.8f));
				mi->Set<float>("Emission", 0.0f);
				mi->Set<float>("Metalness", 0.0f);
				mi->Set<float>("Roughness", 0.8f);
				mi->Set<int>("UseNormalMap", 1);

				mi->Set<TextureUniform>("sampler_MetalnessTexture", Uniform);
				mi->Set<TextureUniform>("sampler_AlbedoTexture", Uniform);
				mi->Set<TextureUniform>("sampler_RoughnessTexture", Uniform);
				mi->Set<TextureUniform>("sampler_NormalTexture", Uniform);

				m_Materials.push_back(mi);
			}
		}
	}

	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
		result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
		result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
		result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
		return result;
	}

	void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
	{
		const glm::mat4 localTransform = Mat4FromAssimpMat4(node->mTransformation);
		const glm::mat4 transform = parentTransform * localTransform;
		m_NodeMap[node].resize(node->mNumMeshes);
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			uint32_t mesh = node->mMeshes[i];
			auto& submesh = m_Submeshes[mesh];
			submesh.NodeName = node->mName.C_Str();
			submesh.Transform = transform;
			submesh.LocalTransform = localTransform;
			m_NodeMap[node][i] = mesh;
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			TraverseNodes(node->mChildren[i], transform, level + 1);
	}


	void Mesh::ResetVertices(float* vertices, uint32_t vertexCount)
	{
		m_Vertices.clear();
		m_Vertices.resize(vertexCount);
		memcpy(m_Vertices.data(), vertices, vertexCount * sizeof(Vertex));
	}

	void Mesh::ResetIndices(uint32_t* indices, uint32_t indexCount)
	{
		m_Indices.clear();
		m_Indices.resize(indexCount);
		memcpy(m_Indices.data(), indices, indexCount * sizeof(uint32_t));
	}

	Ref<Mesh> MeshFactory::Create(PrimitiveType primitiveType)
	{
		switch (primitiveType)
		{
		case PrimitiveType::Plane:					return Plane();
		case PrimitiveType::Quad:					return Quad();
		case PrimitiveType::FullScreenQuad:			return FullScreenQuad();
		case PrimitiveType::Triangle:				return Triangle();
		case PrimitiveType::Cube:					return Cube();
		case PrimitiveType::Sphere:					return Sphere(1.0f);
		case PrimitiveType::TessellatedQuad:		return TessellatedQuad(10);
		}

		return nullptr;
	}

	Ref<Mesh> MeshFactory::Plane()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Triangle()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {0.0f, 0.0f} },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {1.0f, 0.0f} },	// 1 1
			Vertex{ { 0.0f,  0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {0.5f, 1.0f} },	// 2 2
		};

		std::vector<uint32_t> indices = { 0, 1, 2 };

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Quad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::FullScreenQuad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}


	Ref<Mesh> MeshFactory::Cube()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {0.0f, 0.0f}  },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {1.0f, 0.0f}  },	// 1 1
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {1.0f, 1.0f}  },	// 2 2
			Vertex{ {-0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {0.0f, 1.0f}  },	// 3 3

			// Right Face									
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {0.0f, 0.0f}  },	// 1 4 
			Vertex{ { 0.5f, -0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {1.0f, 0.0f}  },	// 5 5
			Vertex{ { 0.5f,  0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {1.0f, 1.0f}  },	// 6 6
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {0.0f, 1.0f}  },	// 2 7

			// Back Face									
			Vertex{ { 0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {0.0f, 0.0f} },	// 4 8
			Vertex{ {-0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {1.0f, 0.0f} },	// 5 9
			Vertex{ {-0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {1.0f, 1.0f} },	// 6 10
			Vertex{ { 0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {0.0f, 1.0f} },	// 7 11

			// Left Face									
			Vertex{ {-0.5f, -0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 0 12
			Vertex{ {-0.5f, -0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 4 13
			Vertex{ {-0.5f,  0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 7 14
			Vertex{ {-0.5f,  0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {0.0f, 1.0f} },	// 3 15

			// Bottom Face									
			Vertex{ { 0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 0 16
			Vertex{ {-0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 1 17
			Vertex{ {-0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 5 18
			Vertex{ { 0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {0.0f, 1.0f} },	// 4 19

			// Top Face										
			Vertex{ {-0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 3 20
			Vertex{ { 0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 2 21
			Vertex{ { 0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 6 22
			Vertex{ {-0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {0.0f, 1.0f} }	// 7 23
		};

		std::vector<uint32_t> indices =
		{
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			4, 5, 6,
			6, 7, 4,
			// back
			8, 9, 10,
			10, 11, 8,
			// left
			12, 13, 14,
			14, 15, 12,
			// bottom
			16, 17, 18,
			18, 19, 16,
			// top
			20, 21, 22,
			22, 23, 20
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Sphere(float radius)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		constexpr float latitudeBands = 30;
		constexpr float longitudeBands = 30;

		for (float latitude = 0.0f; latitude <= latitudeBands; latitude++)
		{
			const float theta = latitude * (float)PI / latitudeBands;
			const float sinTheta = glm::sin(theta);
			const float cosTheta = glm::cos(theta);

			float texT = 1.0f - theta / PI;

			for (float longitude = 0.0f; longitude <= longitudeBands; longitude++)
			{
				const float phi = longitude * 2.0f * (float)PI / longitudeBands;
				const float sinPhi = glm::sin(phi);
				const float cosPhi = glm::cos(phi);

				float texS = 1.0f - (phi / (2 * PI));

				Vertex vertex;
				vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
				vertex.TexCoord = { texS, texT };
				vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
				vertices.push_back(vertex);
			}
		}

		for (uint32_t latitude = 0; latitude < (uint32_t)latitudeBands; latitude++)
		{
			for (uint32_t longitude = 0; longitude < (uint32_t)longitudeBands; longitude++)
			{
				const uint32_t first = (latitude * ((uint32_t)longitudeBands + 1)) + longitude;
				const uint32_t second = first + (uint32_t)longitudeBands + 1;

				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}

		return CreateRef<Mesh>(vertices, indices);
	}

	static int AddIcosphereVertex(const glm::vec3& v, std::vector<Vertex>& vertices, uint32_t* index)
	{
		float length = glm::length(v);
		glm::vec3 unitSphereV = v / length;
		vertices.push_back({unitSphereV, unitSphereV, {}, {}, {0.0f, 0.0f}});
		return (*(index))++;
	}

	static uint32_t GetIcosphereMidpoint(int p1, int p2, std::unordered_map<uint64_t, int>& midpointCache, std::vector<Vertex>& vertices, uint32_t* index)
	{
		bool firstIsSmaller = p1 < p2;
		uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
		uint64_t largerIndex = firstIsSmaller ? p2 : p1;
		uint64_t key = (smallerIndex << 32) + largerIndex;

		if (midpointCache.find(key) != midpointCache.end())
			return midpointCache[key];

		Vertex v1 = vertices[p1];
		Vertex v2 = vertices[p2];
		glm::vec3 mid = (v1.Position + v2.Position) / 2.0f;
		int vertexIndex = AddIcosphereVertex(mid, vertices, index);
		midpointCache[key] = vertexIndex;
		return vertexIndex;
	}

	Ref<Mesh> MeshFactory::Icosphere(uint32_t level, float radius)
	{
		float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;
		
		uint32_t vertexIndex = 0;
		std::vector<Vertex> vertices;

		AddIcosphereVertex({ -1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -1.0f, -t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f, -t, 0.0f }, vertices, &vertexIndex);

		AddIcosphereVertex({ -0.0f, -1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f, -1.0f, -t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f, -t }, vertices, &vertexIndex);

		AddIcosphereVertex({  t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  t, 0.0f,  1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f,  1.0f }, vertices, &vertexIndex);

		struct Triangle { uint32_t A, B, C; };

		std::vector<Triangle> facesOne;

		facesOne.push_back({ 0, 11, 5 });
		facesOne.push_back({ 0, 5, 1 });
		facesOne.push_back({ 0, 1, 7 });
		facesOne.push_back({ 0, 7, 10 });
		facesOne.push_back({ 0, 10, 11 });

		// 5 adjacent faces
		facesOne.push_back({ 1, 5, 9 });
		facesOne.push_back({ 5, 11, 4 });
		facesOne.push_back({ 11, 10, 2 });
		facesOne.push_back({ 10, 7, 6 });
		facesOne.push_back({ 7, 1, 8 });;

		// 5 faces around point 3
		facesOne.push_back({ 3, 9, 4 });
		facesOne.push_back({ 3, 4, 2 });
		facesOne.push_back({ 3, 2, 6 });
		facesOne.push_back({ 3, 6, 8 });
		facesOne.push_back({ 3, 8, 9 });

		// 5 adjacent faces
		facesOne.push_back({ 4, 9, 5 });
		facesOne.push_back({ 2, 4, 11 });
		facesOne.push_back({ 6, 2, 10 });
		facesOne.push_back({ 8, 6, 7 });
		facesOne.push_back({ 9, 8, 1 });

		std::unordered_map<uint64_t, int> midpointCache;

		for (uint32_t i = 0; i < level; i++)
		{
			std::vector<Triangle> facesTwo;

			for (auto triangle : facesOne)
			{
				uint32_t a = GetIcosphereMidpoint(triangle.A, triangle.B, midpointCache, vertices, &vertexIndex);
				uint32_t b = GetIcosphereMidpoint(triangle.B, triangle.C, midpointCache, vertices, &vertexIndex);
				uint32_t c = GetIcosphereMidpoint(triangle.C, triangle.A, midpointCache, vertices, &vertexIndex);

				facesTwo.push_back({ triangle.A, a, c });
				facesTwo.push_back({ triangle.B, b, a });
				facesTwo.push_back({ triangle.C, c, b });
				facesTwo.push_back({ a, b, c });
			}

			facesOne = facesTwo;
		}

		std::vector<uint32_t> indices;
		for (auto triangle : facesOne)
		{
			indices.push_back(triangle.A);
			indices.push_back(triangle.B);
			indices.push_back(triangle.C);
		}

		return CreateRef<Mesh>(vertices, indices);
	}

	std::string MeshFactory::PrimitiveTypeToString(PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::Cube: return "Cube";
		case PrimitiveType::Sphere: return "Sphere";
		case PrimitiveType::Quad: return "Quad";
		case PrimitiveType::Plane: return "Plane";
		case PrimitiveType::FullScreenQuad: return "Full Screen Quad";
		case PrimitiveType::TessellatedQuad: return "Tessellated Quad";
		}

		return "";
	}

	static glm::vec3 RemapVector3(const glm::vec3& inVector)
	{
		float x = Remap(inVector.x, -0.5f, 0.5f, 0.0f, 1.0f);
		float y = Remap(inVector.y, -0.5f, 0.5f, 0.0f, 1.0f);
		float z = Remap(inVector.z, -0.5f, 0.5f, 0.0f, 1.0f);

		return { x, y, z };
	}

	Ref<Mesh> MeshFactory::TessellatedQuad(uint32_t resolution)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Space between each vertex.  This is for a unit quad (-0.5f, -0.5f) - (0.5f, 0.5).
		float spacing = 1.0f / (float)resolution;
		
		glm::vec3 bottomLeft{ -0.5f, -0.5f, 0.0f };

		for (uint32_t y = 0; y < resolution; y++)
		{
			for (uint32_t x = 0; x < resolution; x++)
			{
				glm::vec3 position{ bottomLeft.x + (float)x * spacing, bottomLeft.y + (float)y * spacing, 0.0f };

				glm::vec2 texCoord{ (float)x / resolution, (float)y / resolution };
				glm::vec3 normal{ 0.0f, 0.0f, 1.0f };

				vertices.push_back({ position, normal, {}, {}, texCoord });

				if (x == resolution - 1 || y == resolution - 1) continue;

				uint32_t a = y * resolution + x;
				uint32_t b = y * resolution + x + resolution;
				uint32_t c = y * resolution + x + resolution + 1;
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);

				uint32_t d = a;
				uint32_t e = y * resolution + x + resolution + 1;
				uint32_t f = a + 1;
				indices.push_back(d);
				indices.push_back(e);
				indices.push_back(f);
			}
		}

		return CreateRef<Mesh>(vertices, indices);
	}


}

