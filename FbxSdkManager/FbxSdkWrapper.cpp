#include "FbxSdkWrapper.h"
#include <iostream>

FbxSdkWrapper::FbxSdkWrapper()
    : m_manager(nullptr), m_scene(nullptr), m_loaded(false)
{
    FbxSdkLibrary::InitializeSdkObjects(m_manager, m_scene);
}

FbxSdkWrapper::~FbxSdkWrapper()
{
    if (m_manager)
    {
        FbxSdkLibrary::DestroySdkObjects(m_manager);
    }
}

FbxSdkWrapper::FbxSdkWrapper(FbxSdkWrapper&& other) noexcept
    : m_manager(other.m_manager), m_scene(other.m_scene), m_loaded(other.m_loaded)
{
    other.m_manager = nullptr;
    other.m_scene = nullptr;
    other.m_loaded = false;
}

FbxSdkWrapper& FbxSdkWrapper::operator=(FbxSdkWrapper&& other) noexcept
{
    if (this != &other)
    {
        // 清理当前资源
        if (m_manager)
        {
            FbxSdkLibrary::DestroySdkObjects(m_manager);
        }

        // 移动资源
        m_manager = other.m_manager;
        m_scene = other.m_scene;
        m_loaded = other.m_loaded;

        // 清空源对象
        other.m_manager = nullptr;
        other.m_scene = nullptr;
        other.m_loaded = false;
    }
    return *this;
}

bool FbxSdkWrapper::LoadFile(const std::string& filename)
{
    if (!m_manager || !m_scene)
    {
        std::cerr << "FbxSdkWrapper: Manager or Scene not initialized" << std::endl;
        return false;
    }

    m_loaded = FbxSdkLibrary::LoadScene(m_manager, m_scene, filename.c_str());
    return m_loaded;
}

std::map<std::string, std::string> FbxSdkWrapper::GetMetadata() const
{
    std::map<std::string, std::string> result;
    
    if (!IsLoaded())
    {
        return result;
    }

    std::map<const char*, const char*> rawMetadata;
    FbxSdkLibrary::GetMetaData(m_scene, rawMetadata);

    // 转换为std::string以确保安全
    for (const auto& pair : rawMetadata)
    {
        if (pair.first && pair.second)
        {
            result[pair.first] = pair.second;
        }
    }

    return result;
}

std::map<uint64_t, FbxGeometryInfo> FbxSdkWrapper::GetGeometries() const
{
    if (!IsLoaded())
    {
        return {};
    }

    return FbxSdkLibrary::GetFbxGeometries(m_scene);
}

std::map<uint64_t, FbxMaterialsInfo> FbxSdkWrapper::GetMaterials() const
{
    if (!IsLoaded())
    {
        return {};
    }

    std::map<uint64_t, FbxMaterialsInfo> materials;
    FbxSdkLibrary::GetFbxMaterials(m_scene, materials);
    return materials;
}

// FbxGeometryExporter implementation
std::vector<FbxGeometryExporter::SimplifiedMesh> 
FbxGeometryExporter::ConvertToSimplifiedMeshes(const FbxGeometryInfo& geometryInfo)
{
    std::vector<SimplifiedMesh> meshes;

    // 为每个材质创建一个简化的网格
    for (const auto& sectionPair : geometryInfo.Sections)
    {
        SimplifiedMesh mesh;
        mesh.materialId = sectionPair.first;
        
        const FbxSection& section = sectionPair.second;
        
        // 预分配内存
        size_t vertexCount = section.Triangle.size();
        mesh.vertices.reserve(vertexCount);
        mesh.indices.reserve(vertexCount);

        // 转换顶点数据
        for (size_t i = 0; i < vertexCount; ++i)
        {
            SimplifiedVertex vertex;
            
            // 位置
            int controlPointIndex = section.Triangle[i];
            if (controlPointIndex >= 0 && controlPointIndex < geometryInfo.ControlPoints.size())
            {
                const FbxVector4& pos = geometryInfo.ControlPoints[controlPointIndex];
                vertex.position[0] = static_cast<float>(pos[0]);
                vertex.position[1] = static_cast<float>(pos[1]);
                vertex.position[2] = static_cast<float>(pos[2]);
            }
            
            // 法线
            if (i < section.Normals.size())
            {
                const FbxVector4& normal = section.Normals[i];
                vertex.normal[0] = static_cast<float>(normal[0]);
                vertex.normal[1] = static_cast<float>(normal[1]);
                vertex.normal[2] = static_cast<float>(normal[2]);
            }
            
            // UV
            if (i < section.UVs.size())
            {
                const FbxVector2& uv = section.UVs[i];
                vertex.uv[0] = static_cast<float>(uv[0]);
                vertex.uv[1] = static_cast<float>(uv[1]);
            }
            
            // 顶点颜色
            if (i < section.Colors.size())
            {
                const FbxColor& color = section.Colors[i];
                vertex.color[0] = static_cast<float>(color[0]);
                vertex.color[1] = static_cast<float>(color[1]);
                vertex.color[2] = static_cast<float>(color[2]);
                vertex.color[3] = static_cast<float>(color[3]);
            }
            else
            {
                // 默认白色
                vertex.color[0] = vertex.color[1] = vertex.color[2] = vertex.color[3] = 1.0f;
            }
            
            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(static_cast<uint32_t>(i));
        }
        
        meshes.push_back(std::move(mesh));
    }
    
    return meshes;
}