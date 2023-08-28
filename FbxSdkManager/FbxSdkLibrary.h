#pragma once
#include <fbxsdk.h>
#include <map>
#include <vector>
#include <array>
using namespace std;

typedef array<double,2> Point2D;
typedef array<double,3> Point3D;
typedef array<double,4> RGBA;

typedef vector<Point3D> Polygon3D;


class FbxSdkLibrary
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
    * @brief 获得Mesh的控制点
    */
    static void GetMeshControlPoint(const FbxMesh* pMesh, vector<Point3D>& ControlPoints);
    /**
    * @brief 获得Mesh的Polygon信息
    */
    static void GetMeshPolygonPoints(FbxMesh* pMesh,vector<Polygon3D>& Polygons);
    /**
    * @brief 获得Mesh的顶点颜色信息
    */
    static void GetMeshVertexColor(FbxMesh* pMesh,vector<vector<RGBA>>& PolygonColors);
};

