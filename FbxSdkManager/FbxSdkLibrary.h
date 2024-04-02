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

typedef array<double,2> Point2D;
typedef array<double,3> Point3D;
typedef array<double,4> RGBA;


typedef vector<Point3D> Polygon3D;


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
    static void GetFbxGeometries(FbxScene* const pScene);
    
    /**
    * @brief 获得Mesh的控制点
    */
    static void GetMeshControlPoint(const FbxMesh* pMesh, vector<Point3D>& ControlPoints);
    /**
    * @brief 获得Mesh的顶点颜色信息
    */
    static void GetMeshVertexColor(FbxMesh* pMesh,int PolygonIndex,int ControlPointIndex,RGBA& Color);
    /**
    * @brief 获得Mesh的顶点的UV信息
    */
    static void GetMeshUV(FbxMesh* pMesh, int PolygonIndex, int ControlPointIndex, int PositionInPolygon, Point2D& UV);
    /**
    * @brief 获得Mesh的法线信息
    */
    static void GetMeshNormal(FbxMesh* pMesh, int PolygonIndex, Point3D& Normal);
    /**
    * @brief 获得Mesh的顶点切线信息
    */
    static void GetMeshTangent(FbxMesh* pMesh, int PolygonIndex, Point3D& Tangent);
    /**
    * @brief 获得Mesh的顶点Binormal信息
    */
    static void GetMeshBinormal(FbxMesh* pMesh, int PolygonIndex, Point3D& Binormal);

    /**
    * @brief 获得Mesh的顶点Binormal信息
    */
    static void GetPolygonMaterialId(FbxMesh* pMesh, int PolygonIndex, int& Id);

    static int Test(){return 100;}
    
};

