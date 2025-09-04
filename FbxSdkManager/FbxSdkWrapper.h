#pragma once
#include "FbxSdkLibrary.h"
#include <memory>
#include <string>

/**
 * @brief FBX SDK的RAII封装类，自动管理FbxManager和FbxScene的生命周期
 */
class FbxSdkWrapper
{
public:
    FbxSdkWrapper();
    ~FbxSdkWrapper();

    // 禁用拷贝
    FbxSdkWrapper(const FbxSdkWrapper&) = delete;
    FbxSdkWrapper& operator=(const FbxSdkWrapper&) = delete;

    // 允许移动
    FbxSdkWrapper(FbxSdkWrapper&& other) noexcept;
    FbxSdkWrapper& operator=(FbxSdkWrapper&& other) noexcept;

    /**
     * @brief 加载FBX文件
     * @param filename 文件路径
     * @return 是否成功
     */
    bool LoadFile(const std::string& filename);

    /**
     * @brief 获取场景元数据
     * @return 元数据映射
     */
    std::map<std::string, std::string> GetMetadata() const;

    /**
     * @brief 获取所有几何体信息
     * @return 几何体信息映射
     */
    std::map<uint64_t, FbxGeometryInfo> GetGeometries() const;

    /**
     * @brief 获取所有材质信息
     * @return 材质信息映射
     */
    std::map<uint64_t, FbxMaterialsInfo> GetMaterials() const;

    /**
     * @brief 获取场景指针（用于高级操作）
     */
    FbxScene* GetScene() const { return m_scene; }

    /**
     * @brief 获取管理器指针（用于高级操作）
     */
    FbxManager* GetManager() const { return m_manager; }

    /**
     * @brief 检查是否已加载场景
     */
    bool IsLoaded() const { return m_scene != nullptr && m_loaded; }

private:
    FbxManager* m_manager;
    FbxScene* m_scene;
    bool m_loaded;
};

/**
 * @brief 辅助函数：将FBX文件转换为简化的几何数据
 */
class FbxGeometryExporter
{
public:
    struct SimplifiedVertex
    {
        float position[3];
        float normal[3];
        float uv[2];
        float color[4];
    };

    struct SimplifiedMesh
    {
        std::vector<SimplifiedVertex> vertices;
        std::vector<uint32_t> indices;
        uint64_t materialId;
    };

    /**
     * @brief 将FbxGeometryInfo转换为简化的网格数据
     * @param geometryInfo 输入的几何信息
     * @return 简化的网格数据列表（按材质分组）
     */
    static std::vector<SimplifiedMesh> ConvertToSimplifiedMeshes(const FbxGeometryInfo& geometryInfo);
};