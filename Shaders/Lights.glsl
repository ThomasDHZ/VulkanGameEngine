struct DirectionalLightBuffer
{
    vec3  LightColor;            
    float _pad0;
    vec3  LightDirection;        
    float LightIntensity;       
    float ShadowStrength;        
    float ShadowBias;            
    float ShadowSoftness;       
    uint  LightActive;           
    uint  _pad1;                
    uint  _pad2;                
    uint  _pad3;                
    uint  _pad4;                 
    mat4  LightSpaceMatrix;      
};

struct PointLightBuffer
{
    vec3  LightPosition;        
    float _pad0;
    vec3  LightColor;            
    float LightRadius;         
    float LightIntensity;       
    float ShadowStrength;       
    float ShadowBias;           
    float ShadowSoftness;        
    int   LightLayer;           
    uint  LightActive;          
    uint  _pad1;               
    uint  _pad2;                
    mat4  LightSpaceMatrix;  
};
