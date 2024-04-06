#pragma once
#include <fbxsdk.h>
#include <map>
#include <vector>
#include <array>

#define DLLTEST_EXPORTS
#ifdef DLLTEST_EXPORTS
# define DLL_API _declspec(dllexport)
# else
# define DLL_API _declspec(dllimport)

#endif // DLLTEST_EXPORTS
using namespace std;



struct FbxMaterialColorProperty
{
 FbxDouble3 Color;
 const char* Texture;
};

struct FbxMaterialFactorProperty
{
 double Factory;
 const char* Texture;
};

struct FbxMaterialsInfo
{
 FbxMaterialColorProperty Ambient;
 FbxMaterialColorProperty Diffuse;
 FbxMaterialColorProperty Specular;
 FbxMaterialColorProperty Emissive;
 FbxMaterialFactorProperty Opacity;
 FbxMaterialFactorProperty Shininess;
 FbxMaterialFactorProperty Reflectivity;
};

struct FbxSection
{
   vector<int> Triangle;
   vector<FbxColor> Colors;
   vector<FbxVector2> UVs;
   vector<FbxVector4> Normals;
   vector<FbxVector4> Tangents;
   vector<FbxVector4> Binormals;
   uint64_t MaterialIds;
};

struct FbxNodeInfo
{
 uint64_t ParentId = 0;
 uint64_t Id;
 const char* NodeName;
 uint64_t LinkMeshId = 0;
 vector<uint64_t> LinkMaterialsId;
 vector<map<const char*,const char*>> Metadata;
};

class DLL_API FbxSdkLibrary
{
public:
    /**
     * @brief 初始化FbxManager
     */
    static void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
    /**
     * @brief 销毁FbxManager
     */
    static void DestroySdkObjects(FbxManager* pManager);
    /**
    * @brief 销毁FbxManager
    */
    static bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
    /**
    * @brief 获得Fbx文件的metadata
    */
    static void GetMetaData(FbxScene* pScene,map<string, string>& MetaData);
    /**
    * @brief 获得Scene里面的所有Geometry
    */
    static map<uint64_t, map<uint64_t, FbxSection>> GetFbxGeometries(FbxScene* const pScene);
    
    /**
    * @brief 获得Mesh的控制点
    */
    static void GetMeshControlPoint(const FbxMesh* pMesh, vector<FbxVector4>& ControlPoints);
   /**
    * @brief 获得Mesh的顶点颜色信息
    */
    static void GetPolygonVertexColor(FbxMesh* pMesh,int PolygonIndex,int ControlPointIndex,FbxColor& Color);
    /**
    * @brief 获得Mesh的顶点的UV信息
    */
    static void GetPolygonUV(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex, int PositionInPolygon, FbxVector2& UV);
    /**
    * @brief 获得Mesh的法线信息
    */
    static void GetPolygonNormal(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Normal);
    /**
    * @brief 获得Mesh的顶点切线信息
    */
    static void GetPolygonTangent(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Tangent);
    /**
    * @brief 获得Mesh的顶点Binormal信息
    */
    static void GetPolygonBinormal(FbxMesh* pMesh, int PolygonIndex, FbxVector4& Binormal);

    /**
    * @brief 获得Polygon对应的材质ID
    */
    static void GetPolygonMaterialId(FbxMesh* pMesh, int PolygonIndex, uint64_t& Id);
    /**
     *@brief 获得场景材质信息
     */
    static void GetFbxMaterials(FbxScene* pScene,map<uint64_t,FbxMaterialsInfo>& MaterialInfos);

    static const char* GetMaterialTexture(FbxSurfaceMaterial* pMaterial,const char* Property);
 
    static int Test(){return 100;}
    
};

