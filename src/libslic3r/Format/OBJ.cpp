#include "../libslic3r.h"
#include "../Model.hpp"
#include "../TriangleMesh.hpp"


#include "OBJ.hpp"
#include "ResourcePathUtils.hpp"
#include "objparser.hpp"

#include <string>
#include <fstream>
#include <map>

#include <boost/log/trivial.hpp>
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

//Translation
#include "I18N.hpp"
#define _L(s) Slic3r::I18N::translate(s)

namespace Slic3r {

bool load_obj(const char *path, TriangleMesh *meshptr, ObjInfo &obj_info, std::string &message, bool gamma_correct, ObjParser::MtlData *out_mtl)
{
    if (meshptr == nullptr)
        return false;
    // Parse the OBJ file.
    ObjParser::ObjData data;
    ObjParser::MtlData mtl_data;
    if (! ObjParser::objparse(path, data)) {
        BOOST_LOG_TRIVIAL(error) << "load_obj: failed to parse " << path;
        message = _L("load_obj: failed to parse");
        return false;
    }

    obj_info.ml_region = data.ml_region;
    obj_info.ml_name = data.ml_name;
    obj_info.ml_id = data.ml_id;

    bool exist_mtl = false;
    if (data.mtllibs.size() > 0) { // read mtl
        for (auto mtl_name : data.mtllibs) {
            if (mtl_name.size() == 0){
                continue;
            }
            exist_mtl = true;
            std::wstring   wide_mtl_name = boost::locale::conv::to_utf<wchar_t>(mtl_name, "UTF-8");
            if (boost::istarts_with(wide_mtl_name,"./")){
                boost::replace_first(wide_mtl_name, "./", "");
            }
            const boost::filesystem::path requested_mtl_path(wide_mtl_name);
            const boost::filesystem::path obj_dir = boost::filesystem::path(path).parent_path();
            const boost::filesystem::path mtl_path = requested_mtl_path.is_absolute() ?
                resource_path::resolve_existing_path_case_insensitive(requested_mtl_path, "load_obj: mtllib") :
                resource_path::resolve_existing_relative_path_case_insensitive(obj_dir, requested_mtl_path, "load_obj: mtllib");

            if (!mtl_path.empty()) {
                const std::string mtl_path_string = mtl_path.string();
                if (!ObjParser::mtlparse(mtl_path_string.c_str(), mtl_data)) {
                    BOOST_LOG_TRIVIAL(error) << "load_obj:load_mtl: failed to parse " << mtl_path_string;
                    message = _L("load mtl in obj: failed to parse");
                    return false;
                }
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "load_obj: failed to load mtl_path:" << requested_mtl_path;
            }
        }
    }
    // Count the generated triangles and verify that all faces are valid polygons.
    size_t num_triangles = 0;
    for (size_t i = 0; i < data.vertices.size(); ++ i) {
        // Find the end of face.
        size_t j = i;
        for (; j < data.vertices.size() && data.vertices[j].coordIdx != -1; ++ j) ;
        if (size_t num_face_vertices = j - i; num_face_vertices > 0) {
            if (num_face_vertices < 3) {
                BOOST_LOG_TRIVIAL(error) << "load_obj: failed to parse " << path << ". The file contains polygons with less than 3 vertices.";
                message = _L("The file contains polygons with less than 3 vertices.");
                return false;
            }
            num_triangles += num_face_vertices - 2;
            i = j;
        }
    }
    // Convert ObjData into indexed triangle set.
    indexed_triangle_set its;
    size_t               num_vertices = data.coordinates.size() / OBJ_VERTEX_LENGTH;
    its.vertices.reserve(num_vertices);
    its.indices.reserve(num_triangles);
    if (exist_mtl) {
        obj_info.is_single_mtl = data.usemtls.size() == 1 && mtl_data.new_mtl_unmap.size() == 1;
        obj_info.face_colors.reserve(num_triangles);
        obj_info.uvs.reserve(num_triangles);
        obj_info.usemtls = data.usemtls;
    }
    for (size_t i = 0; i < num_vertices; ++ i) {
        size_t j = i * OBJ_VERTEX_LENGTH;
        its.vertices.emplace_back(data.coordinates[j], data.coordinates[j + 1], data.coordinates[j + 2]);
        if (data.has_vertex_color) {
            RGBA color{data.coordinates[j + 3], data.coordinates[j + 4], data.coordinates[j + 5],data.coordinates[j + 6]};
            if (gamma_correct) {
                ColorRGBA::gamma_correct(color);
            }
            for (int i = 0; i < color.size(); i++) {
                color[i] = std::clamp(color[i], 0.f, 1.f);
            }
            obj_info.vertex_colors.emplace_back(color);
        }
    }
    for (size_t i = 0; i < data.vertices.size();)
        if (data.vertices[i].coordIdx == -1)
            ++ i;
        else {
            std::vector<int> indices;
            std::vector<int> uvs;
            while (i < data.vertices.size())
                if (const ObjParser::ObjVertex &vertex = data.vertices[i ++]; vertex.coordIdx == -1) {
                    break;
                } else {
                    if (vertex.coordIdx < 0 || vertex.coordIdx >= int(its.vertices.size())) {
                        BOOST_LOG_TRIVIAL(error) << "load_obj: failed to parse " << path << ". The file contains invalid vertex index.";
                        message = _L("The file contains invalid vertex index.");
                        return false;
                    }
                    // Reject UV indices outside the parsed texture coordinate table up front, so get_uv
                    // below never sees a value large enough to overflow its signed bounds check.
                    // -1 is the "no texture coordinate" marker (e.g. faces written as "f v//n"),
                    // and with an empty table the UV references are inert (the get_uv block below
                    // is skipped), so both keep loading as they did before.
                    if (data.textureCoordinates.size() > 0 && vertex.textureCoordIdx != -1 &&
                        (vertex.textureCoordIdx < 0 || size_t(vertex.textureCoordIdx) * 2 + 1 >= data.textureCoordinates.size())) {
                        BOOST_LOG_TRIVIAL(error) << "load_obj: failed to parse " << path << ". The file contains invalid texture coordinate index.";
                        message = _L("The file contains invalid texture coordinate index.");
                        return false;
                    }
                    indices.push_back(vertex.coordIdx);
                    uvs.push_back(vertex.textureCoordIdx);
                }
            if (!indices.empty()) {
                auto get_face_color = [&mtl_data ,& gamma_correct](const std::string mtl_name, RGBA &face_color) {
                    if (mtl_data.new_mtl_unmap.find(mtl_name) != mtl_data.new_mtl_unmap.end()) {
                        for (size_t n = 0; n < 3; n++) { // 0.1 is light ambient
                            float object_ka = 0.f;
                            if (mtl_data.new_mtl_unmap[mtl_name]->Ka[n] > 0.01 && mtl_data.new_mtl_unmap[mtl_name]->Ka[n] < 0.99) {
                                object_ka = mtl_data.new_mtl_unmap[mtl_name]->Ka[n] * 0.1;
                            }
                            auto  value   = object_ka + float(mtl_data.new_mtl_unmap[mtl_name]->Kd[n]);
                            float temp    = gamma_correct ? ColorRGBA::gamma_correct(value) : value;
                            face_color[n] = std::clamp(temp, 0.f, 1.f);
                        }
                        face_color[3] = gamma_correct ? ColorRGBA::gamma_correct(mtl_data.new_mtl_unmap[mtl_name]->Tr) : mtl_data.new_mtl_unmap[mtl_name]->Tr; // alpha
                        return true;
                    }
                    return false;
                };
                auto set_face_color = [&data, &mtl_data, &obj_info, &get_face_color](int face_index, const std::string mtl_name, const std::array<int, 3>& tri_uvs) {
                    if (mtl_data.new_mtl_unmap.find(mtl_name) != mtl_data.new_mtl_unmap.end()) {
                        RGBA face_color;
                        get_face_color(mtl_name,face_color);
                        if (mtl_data.new_mtl_unmap[mtl_name]->map_Kd.size() > 0) {
                            auto png_name       = mtl_data.new_mtl_unmap[mtl_name]->map_Kd;
                            obj_info.has_uv_png = true;
                            if (obj_info.pngs.find(png_name) == obj_info.pngs.end()) { obj_info.pngs[png_name] = false; }
                            obj_info.uv_map_pngs[face_index] = png_name;
                        }
                        if (data.textureCoordinates.size() > 0) {
                            auto get_uv = [&data](int uv_idx) {
                                if (uv_idx < 0 || size_t(uv_idx) * 2 + 1 >= data.textureCoordinates.size())
                                    return Vec2f(0.f, 0.f);
                                return Vec2f(data.textureCoordinates[size_t(uv_idx) * 2], data.textureCoordinates[size_t(uv_idx) * 2 + 1]);
                            };
                            Vec2f                uv0 = get_uv(tri_uvs[0]);
                            Vec2f                uv1 = get_uv(tri_uvs[1]);
                            Vec2f                uv2 = get_uv(tri_uvs[2]);
                            std::array<Vec2f, 3> uv_array{uv0, uv1, uv2};
                            obj_info.uvs.emplace_back(uv_array);
                        }
                        obj_info.face_colors.emplace_back(face_color);
                    }
                    else {
                        if (obj_info.lost_material_name.empty()) {
                            obj_info.lost_material_name = mtl_name;
                        }
                    }
                };
                auto set_face_color_by_mtl = [&data, &set_face_color](int face_index, const std::array<int, 3>& tri_uvs) {
                    if (data.usemtls.size() == 1) {
                        set_face_color(face_index, data.usemtls[0].name, tri_uvs);
                    } else {
                        for (size_t k = 0; k < data.usemtls.size(); k++) {
                            auto mtl = data.usemtls[k];
                            if (face_index >= mtl.face_start && face_index <= mtl.face_end) {
                                set_face_color(face_index, data.usemtls[k].name, tri_uvs);
                                break;
                            }
                        }
                    }
                };
                if (exist_mtl) {
                    if (obj_info.mtl_colors.empty()) {
                        if (mtl_data.first_time_using_makerlab) {
                            obj_info.first_time_using_makerlab = true;
                        }
                        obj_info.mtl_colors.reserve(mtl_data.mtl_orders.size());
                        obj_info.mtl_color_names.reserve(mtl_data.mtl_orders.size());
                        for (int i = 0; i < mtl_data.mtl_orders.size(); i++) {
                            RGBA face_color;
                            if (get_face_color(mtl_data.mtl_orders[i],face_color)) {
                                obj_info.mtl_colors.emplace_back(face_color);
                                obj_info.mtl_color_names.emplace_back(mtl_data.mtl_orders[i]);
                            }
                        }
                    }
                }

                for (size_t tri = 0; tri + 2 < indices.size(); ++tri) {
                    its.indices.emplace_back(indices[0], indices[tri + 1], indices[tri + 2]);
                    int face_index = static_cast<int>(its.indices.size()) - 1;
                    if (exist_mtl) {
                        const std::array<int, 3> tri_uvs{uvs[0], uvs[tri + 1], uvs[tri + 2]};
                        set_face_color_by_mtl(face_index, tri_uvs);
                    }
                }
            }
        }

    *meshptr = TriangleMesh(std::move(its));
    if (meshptr->empty()) {
        BOOST_LOG_TRIVIAL(error) << "load_obj: This OBJ file couldn't be read because it's empty. " << path;
        message = _L("This OBJ file couldn't be read because it's empty.");
        return false;
    }
    if (meshptr->volume() < 0)
        meshptr->flip_triangles();
    if (out_mtl)
        *out_mtl = mtl_data;
    return true;
}

bool load_obj(const char *path, Model *model, ObjInfo& obj_info, std::string &message, const char *object_name_in, bool gamma_correct, ObjParser::MtlData *out_mtl)
{
    TriangleMesh mesh;

    bool ret = load_obj(path, &mesh, obj_info, message, gamma_correct, out_mtl);

    if (ret) {
        std::string  object_name;
        if (object_name_in == nullptr) {
            const char *last_slash = strrchr(path, DIR_SEPARATOR);
            object_name.assign((last_slash == nullptr) ? path : last_slash + 1);
        } else
           object_name.assign(object_name_in);
        model->add_object(object_name.c_str(), path, std::move(mesh));
    }

    return ret;
}

bool store_obj(const char *path, TriangleMesh *mesh)
{
    //FIXME returning false even if write failed.
    mesh->WriteOBJFile(path);
    return true;
}

bool store_obj(const char *path, ModelObject *model_object)
{
    TriangleMesh mesh = model_object->mesh();
    return store_obj(path, &mesh);
}

bool store_obj(const char *path, Model *model)
{
    TriangleMesh mesh = model->mesh();
    return store_obj(path, &mesh);
}

bool obj_to_textured_mesh(
    const ObjInfo& obj_info,
    const indexed_triangle_set& its,
    const ObjParser::MtlData& mtl_data,
    const std::string& obj_directory,
    TexturedMesh& out)
{
    (void)obj_info; (void)its; (void)mtl_data; (void)obj_directory; (void)out;
    return false;
}

}; // namespace Slic3r
