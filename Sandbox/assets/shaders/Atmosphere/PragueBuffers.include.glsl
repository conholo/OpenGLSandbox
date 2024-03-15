layout(rgba32f, location = 0) restrict writeonly uniform imageCube o_Cubemap;

layout(std430, binding = 1) buffer RadianceData
{
	float radData[];
};

layout(std430, binding = 2) buffer SunMetaData
{
	double sunBreaks[];
};

layout(std430, binding = 3) buffer ZenithMetaData
{
	double zenithBreaks[];
};

layout(std430, binding = 4) buffer EmphMetaData
{
	double emphBreaks[];
};

layout(std430, binding = 5) buffer VisibilitiesData
{
	double visibilitiesRad[];
};

layout(std430, binding = 6) buffer AlbedosData
{
	double albedosRad[];
};

layout(std430, binding = 7) buffer AltitudesData
{
	double altitudesRad[];
};

layout(std430, binding = 8) buffer ElevationsData
{
	double elevationsRad[];
};


layout(std140, binding = 13) uniform Input
{
	vec4  u_OriginData;
	float u_Albedo;
	float u_Altitude;
	float u_Azimuth;
	float u_Elevation;
	float u_Visibility;
};

layout(std140, binding = 14) uniform DatasetUniforms
{
	float	u_ChannelStart;
	float	u_ChannelWidth;
	int		u_aDim;
	int		u_dDim;
	int		u_RankTrans;
	int		u_Channels;
	int	    u_TotalCoefsSingleConfigRad;
	int	    u_TotalCoefsAllConfigsRad;
	int	    u_RadRank;
	int		sunOffset;
	int		sunStride;
	int		zenithOffset;
	int		zenithStride;
	int		emphOffset;
};