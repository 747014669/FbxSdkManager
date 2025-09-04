#pragma once
#include <fbxsdk.h>
#include <map>
#include <vector>

#define DLLTEST_EXPORTS
#ifdef DLLTEST_EXPORTS
# define DLL_API _declspec(dllexport)
# else
# define DLL_API _declspec(dllimport)

#endif // DLLTEST_EXPORTS
// 避免在头文件中使用 using namespace std;
// 使用具体的类型声明



struct FbxMaterialColorProperty
{
 FbxDouble3 Color;
 const char* Texture;  // 注意：这里存储的是指向FBX SDK内部字符串的指针，生命周期由SDK管理
};

struct FbxMaterialFactorProperty
{
 double Factor;  // 材质因子（如透明度、光泽度等）
 const char* Texture;  // 注意：这里存储的是指向FBX SDK内部字符串的指针
};

struct FbxSection
{
 std::vector<int> Triangle;
 std::vector<FbxColor> Colors;
 std::vector<FbxVector2> UVs;
 std::vector<FbxVector4> Normals;
 std::vector<FbxVector4> Tangents;
 std::vector<FbxVector4> Binormals;
};

struct FbxGeometryInfo
{
 std::vector<FbxVector4> ControlPoints;
 std::map<uint64_t,FbxSection> Sections;
 
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



struct FbxNodeInfo
{
 uint64_t ParentId = 0;
 uint64_t Id;
 const char* NodeName;
 uint64_t LinkMeshId = 0;
 std::vector<uint64_t> LinkMaterialsId;
 std::vector<std::map<const char*,const char*>> Metadata;
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
    static void GetMetaData(FbxScene* pScene, std::map<const char*, const char*>& MetaData);
    /**
    * @brief 获得Scene里面的所有Geometry
    */
    static std::map<uint64_t, FbxGeometryInfo> GetFbxGeometries(FbxScene* pScene);
    
    /**
    * @brief 获得Mesh的控制点
    */
    static void GetMeshControlPoint(const FbxMesh* pMesh, std::vector<FbxVector4>& ControlPoints);
   /**
    * @brief 获得Mesh的顶点颜色信息
    */
    static FbxColor GetPolygonVertexColor(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex);
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
    static void GetFbxMaterials(FbxScene* pScene,std::map<uint64_t,FbxMaterialsInfo>& MaterialInfos);

    static const char* GetMaterialTexture(FbxSurfaceMaterial* pMaterial,const char* Property);

    static const char* test();

private:

};

