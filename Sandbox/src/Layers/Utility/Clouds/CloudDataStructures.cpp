#include "Engine/Core/Random.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Core/Time.h"

#include "HelperFunctions.h"
#include "CloudDataStructures.h"

static Engine::Ref<WorleyChannelData> CreateChannelData(WorleyChannelMask mask, const glm::ivec3& defaultCells, float persistence)
{
	Engine::Ref<WorleyChannelData> data = Engine::CreateRef<WorleyChannelData>();
	data->Mask = mask;

	data->WorleyLayerPersistence = persistence;
	data->LayerSeeds.x = Engine::Random::RandomRange(0.0f, 10e6f);
	data->LayerSeeds.y = Engine::Random::RandomRange(0.0f, 10e6f);
	data->LayerSeeds.z = Engine::Random::RandomRange(0.0f, 10e6f);
	data->LayerCells = defaultCells;
	data->MinMax.resize(2);
	data->MinMax[0] = INT_MAX;
	data->MinMax[1] = INT_MIN;

	data->PointsA = CreateWorleyPoints(data->LayerCells.x, data->LayerSeeds.x);
	data->ShapePointsBufferA = Engine::CreateRef<Engine::ShaderStorageBuffer>(data->PointsA.data(), sizeof(glm::vec4) * data->PointsA.size());
	data->PointsB = CreateWorleyPoints(data->LayerCells.y, data->LayerSeeds.y);
	data->ShapePointsBufferB = Engine::CreateRef<Engine::ShaderStorageBuffer>(data->PointsB.data(), sizeof(glm::vec4) * data->PointsB.size());
	data->PointsC = CreateWorleyPoints(data->LayerCells.z, data->LayerSeeds.z);
	data->ShapePointsBufferC = Engine::CreateRef<Engine::ShaderStorageBuffer>(data->PointsC.data(), sizeof(glm::vec4) * data->PointsC.size());
	data->MinMaxBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(data->MinMax.data(), sizeof(int) * 2);

	return data;
}

BaseShapeWorleySettings::BaseShapeWorleySettings()
{
	ChannelR = CreateChannelData(WorleyChannelMask::R, DefaultLayerCellsR, DefaultPersistence.r);
	ChannelG = CreateChannelData(WorleyChannelMask::G, DefaultLayerCellsG, DefaultPersistence.g);
	ChannelB = CreateChannelData(WorleyChannelMask::B, DefaultLayerCellsB, DefaultPersistence.b);
	ChannelA = CreateChannelData(WorleyChannelMask::A, DefaultLayerCellsA, DefaultPersistence.a);

	Engine::Texture3DSpecification baseShapeTextureSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		ShapeResolution, ShapeResolution, ShapeResolution
	};

	BaseShapeTexture = Engine::CreateRef<Engine::Texture3D>(baseShapeTextureSpec);
}

void BaseShapeWorleySettings::UpdateChannel(WorleyChannelMask mask, const Engine::Ref<Engine::Texture2D>& perlinTexture)
{
	BaseShapeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	uint32_t threadGroups = glm::ceil(ShapeResolution / (float)ShapeThreadGroupSize);

	switch (mask)
	{
	case WorleyChannelMask::R: ChannelR->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
	case WorleyChannelMask::G: ChannelG->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
	case WorleyChannelMask::B: ChannelB->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
	case WorleyChannelMask::A: ChannelA->UpdateChannel(perlinTexture, PerlinWorleyMix, threadGroups); break;
	default: break;
	}
}

void BaseShapeWorleySettings::UpdateAllChannels(const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	BaseShapeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	uint32_t threadGroups = glm::ceil(ShapeResolution / (float)ShapeThreadGroupSize);

	ChannelR->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
	ChannelG->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
	ChannelB->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
	ChannelA->UpdateChannel(perlinSettings->PerlinTexture, PerlinWorleyMix, threadGroups);
}

void WorleyChannelData::UpdatePoints()
{
	LayerSeeds.x = Engine::Random::RandomRange(0.0f, 10e6f);
	LayerSeeds.y = Engine::Random::RandomRange(0.0f, 10e6f);
	LayerSeeds.z = Engine::Random::RandomRange(0.0f, 10e6f);
	PointsA = CreateWorleyPoints(LayerCells.x, LayerSeeds.x);
	PointsB = CreateWorleyPoints(LayerCells.y, LayerSeeds.y);
	PointsC = CreateWorleyPoints(LayerCells.z, LayerSeeds.z);

	ShapePointsBufferA = Engine::CreateRef<Engine::ShaderStorageBuffer>(PointsA.data(), sizeof(glm::vec4) * PointsA.size());
	ShapePointsBufferB = Engine::CreateRef<Engine::ShaderStorageBuffer>(PointsB.data(), sizeof(glm::vec4) * PointsB.size());
	ShapePointsBufferC = Engine::CreateRef<Engine::ShaderStorageBuffer>(PointsC.data(), sizeof(glm::vec4) * PointsC.size());
}

void WorleyChannelData::UpdateChannel(const Engine::Ref<Engine::Texture2D>& perlinTexture, float perlinWorleyMix, uint32_t threadGroups)
{
	MinMax[0] = INT_MAX;
	MinMax[1] = INT_MIN;
	MinMaxBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(MinMax.data(), sizeof(int) * 2);

	Engine::ShaderLibrary::Get("WorleyGenerator")->Bind();
	perlinTexture->BindToSamplerSlot(0);

	ShapePointsBufferA->BindToComputeShader(0, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	ShapePointsBufferB->BindToComputeShader(1, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	ShapePointsBufferC->BindToComputeShader(2, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	MinMaxBuffer->BindToComputeShader(3, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_IsBaseShape", true);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_Invert", InvertWorley);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", LayerCells.x);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", LayerCells.y);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", LayerCells.z);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", WorleyTiling);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_InversionWeight", InversionWeight);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_PerlinWorleyMix", perlinWorleyMix);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", WorleyLayerPersistence);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_PerlinTexture", 0);
	Engine::ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();

	Engine::ShaderLibrary::Get("NormalizeWorley")->Bind();
	MinMaxBuffer->BindToComputeShader(0, Engine::ShaderLibrary::Get("NormalizeWorley")->GetID());
	Engine::ShaderLibrary::Get("NormalizeWorley")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
	Engine::ShaderLibrary::Get("NormalizeWorley")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("NormalizeWorley")->EnableShaderImageAccessBarrierBit();
}

void WorleyChannelData::UpdateChannel(uint32_t threadGroups)
{
	MinMax[0] = INT_MAX;
	MinMax[1] = INT_MIN;
	MinMaxBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(MinMax.data(), sizeof(int) * 2);

	Engine::ShaderLibrary::Get("WorleyGenerator")->Bind();

	ShapePointsBufferA->BindToComputeShader(0, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	ShapePointsBufferB->BindToComputeShader(1, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	ShapePointsBufferC->BindToComputeShader(2, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	MinMaxBuffer->BindToComputeShader(3, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_IsBaseShape", false);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_Invert", InvertWorley);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", LayerCells.x);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", LayerCells.y);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", LayerCells.z);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", WorleyTiling);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_InversionWeight", InversionWeight);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", WorleyLayerPersistence);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_PerlinTexture", 0);
	Engine::ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();

	Engine::ShaderLibrary::Get("NormalizeWorley")->Bind();
	MinMaxBuffer->BindToComputeShader(0, Engine::ShaderLibrary::Get("NormalizeWorley")->GetID());
	Engine::ShaderLibrary::Get("NormalizeWorley")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(Mask));
	Engine::ShaderLibrary::Get("NormalizeWorley")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("NormalizeWorley")->EnableShaderImageAccessBarrierBit();
}

WorleyPerlinSettings::WorleyPerlinSettings()
{
	RandomPerlinOffsets = GeneratePerlinOffsets(Octaves);
	RandomPerlinOffsetsBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(RandomPerlinOffsets.data(), sizeof(glm::vec4) * RandomPerlinOffsets.size());

	Engine::Texture2DSpecification perlinSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		PerlinResolution, PerlinResolution
	};

	PerlinTexture = Engine::CreateRef<Engine::Texture2D>(perlinSpec);
	PerlinTexture->BindToImageSlot(1, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	UpdateTexture();
}

void WorleyPerlinSettings::UpdatePoints()
{
	RandomPerlinOffsets = GeneratePerlinOffsets(Octaves);
	RandomPerlinOffsetsBuffer->SetData(RandomPerlinOffsets.data(), 0, RandomPerlinOffsets.size() * sizeof(glm::vec4));
}

void WorleyPerlinSettings::UpdateTexture()
{
	uint32_t threadGroups = glm::ceil(PerlinResolution / (float)PerlinThreadGroupSize);
	RandomPerlinOffsetsBuffer->BindToComputeShader(3, Engine::ShaderLibrary::Get("Perlin2D")->GetID());
	Engine::ShaderLibrary::Get("Perlin2D")->Bind();
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.NoiseScale", NoiseScale);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Lacunarity", Lacunarity);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Persistence", Persistence);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformInt("u_Settings.Octaves", Octaves);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat2("u_Settings.TextureOffset", TextureOffset);
	Engine::ShaderLibrary::Get("Perlin2D")->DispatchCompute(threadGroups, threadGroups, 1);
	Engine::ShaderLibrary::Get("Perlin2D")->EnableShaderImageAccessBarrierBit();
}

DetailShapeWorleySettings::DetailShapeWorleySettings()
{
	ChannelR = CreateChannelData(WorleyChannelMask::R, DefaultLayerCellsR, DefaultPersistence.r);
	ChannelG = CreateChannelData(WorleyChannelMask::G, DefaultLayerCellsG, DefaultPersistence.g);
	ChannelB = CreateChannelData(WorleyChannelMask::B, DefaultLayerCellsB, DefaultPersistence.b);

	ChannelR->InvertWorley = false;
	ChannelR->InvertWorley = false;
	ChannelR->InvertWorley = false;

	Engine::Texture3DSpecification detailShapeTextureSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		DetailResolution, DetailResolution, DetailResolution
	};

	DetailShapeTexture = Engine::CreateRef<Engine::Texture3D>(detailShapeTextureSpec);
}

void DetailShapeWorleySettings::UpdateChannel(WorleyChannelMask mask)
{
	DetailShapeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	uint32_t threadGroups = glm::ceil(DetailResolution / (float)DetailThreadGroupSize);

	switch (mask)
	{
	case WorleyChannelMask::R: ChannelR->UpdateChannel(threadGroups); break;
	case WorleyChannelMask::G: ChannelG->UpdateChannel(threadGroups); break;
	case WorleyChannelMask::B: ChannelB->UpdateChannel(threadGroups); break;
	default: break;
	}
}

void DetailShapeWorleySettings::UpdateAllChannels()
{
	DetailShapeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	uint32_t threadGroups = glm::ceil(DetailResolution / (float)DetailThreadGroupSize);

	ChannelR->UpdateChannel(threadGroups);
	ChannelG->UpdateChannel(threadGroups);
	ChannelB->UpdateChannel(threadGroups);
}

CurlSettings::CurlSettings()
{
	Engine::Texture2DSpecification curlSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		CurlResolution, CurlResolution
	};

	CurlTexture = Engine::CreateRef<Engine::Texture2D>(curlSpec);

	UpdateTexture();
}

void CurlSettings::UpdateTexture()
{
	CurlTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	uint32_t threadGroups = glm::ceil(CurlResolution / (float)CurlThreadGroupSize);
	Engine::ShaderLibrary::Get("Curl")->Bind();
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Strength", Strength);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Tiling", Tiling);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat2("u_TilingOffset", TilingOffset);
	Engine::ShaderLibrary::Get("Curl")->DispatchCompute(threadGroups, threadGroups, 1);
	Engine::ShaderLibrary::Get("Curl")->EnableShaderImageAccessBarrierBit();
}
