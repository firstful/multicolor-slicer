// multicolor-slicer: thin libslic3r CLI for 3mf/STL -> multi-color gcode.
//
// Subset extraction of BambuStudio's CLI slice path:
//   no GUI (no wx/OpenGL/GLEW/glfw), no OpenCV/OCCT/assimp/CGAL/OpenVDB.
//
// The multi-colour behaviour mirrors BambuStudio.cpp's CLI::run():
//   * plate config from the 3mf's Metadata/project_settings.config is applied
//   * plate filament_maps are applied so AMS slot mapping is preserved
//   * has_filament_switcher is NOT forced off (the upstream CLI bug that
//     collapsed multi-filament slicing to a single tool)

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <boost/nowide/iostream.hpp>

#include "libslic3r/Config.hpp"
#include "libslic3r/Model.hpp"
#include "libslic3r/Print.hpp"
#include "libslic3r/Semver.hpp"
#include "libslic3r/Format/bbs_3mf.hpp"
#include "libslic3r/GCode/GCodeProcessor.hpp"

using namespace Slic3r;

static void usage(const char* argv0)
{
    boost::nowide::cerr << "usage: " << argv0 << " <input.3mf|input.stl> <output.gcode> [plate_index]\n"
                         << "  plate_index defaults to 0 (first plate)\n";
}

// Apply the plate's filament map (1-based extruder ids) to the print config.
static void apply_filament_map(DynamicPrintConfig& config, const std::vector<int>& filament_maps)
{
    if (filament_maps.empty())
        return;

    if (!config.has("filament_map"))
        return;

    std::vector<int> maps = filament_maps;
    // Extruder ids in filament_map are 1-based; guard against out-of-range values
    // so a malformed 3mf cannot produce an invalid config.
    int max_extruder = 1;
    if (config.has("extruder"))
        max_extruder = std::max<int>(1, static_cast<int>(config.option<ConfigOptionInts>("extruder")->values.size()));

    for (int& m : maps) {
        if (m < 1)
            m = 1;
        if (m > max_extruder)
            m = max_extruder;
    }
    config.option<ConfigOptionInts>("filament_map", true)->values = maps;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage(argc > 0 ? argv[0] : "multicolor-slicer");
        return 1;
    }
    const std::string input   = argv[1];
    const std::string output  = argv[2];
    const int plate_index     = (argc > 3) ? std::atoi(argv[3]) : 0;

    DynamicPrintConfig config;
    ConfigSubstitutionContext config_substitutions(ForwardCompatibilitySubstitutionRule::Enable);
    LoadStrategy strategy = LoadStrategy::LoadModel | LoadStrategy::LoadConfig
                          | LoadStrategy::AddDefaultInstances | LoadStrategy::LoadAuxiliary;
    PlateDataPtrs plate_data;
    bool is_bbl_3mf = false;
    Semver file_version;

    Model model = Model::read_from_file(input, &config, &config_substitutions, strategy,
                                        &plate_data, nullptr, &is_bbl_3mf, &file_version,
                                        nullptr, nullptr, nullptr, 0, nullptr);

    if (model.objects.empty()) {
        boost::nowide::cerr << "error: no objects loaded from " << input << "\n";
        return 2;
    }

    // Apply the plate's project config (machine profile + per-plate overrides) on top of
    // the model-level config, exactly like BambuStudio.cpp's CLI slice path does.
    PlateData* plate = nullptr;
    if (plate_index >= 0 && static_cast<size_t>(plate_index) < plate_data.size())
        plate = plate_data[plate_index];
    else if (!plate_data.empty())
        plate = plate_data[0];

    DynamicPrintConfig print_config = config;
    if (plate) {
        print_config.apply(plate->config, true);
        apply_filament_map(print_config, plate->filament_maps);
    }

    // Upstream CLI forces enable_filament_dynamic_map off unless explicitly given.
    // Honour the same default here so old 3mfs with it enabled are handled.
    if (!print_config.has("enable_filament_dynamic_map"))
        print_config.set_key_value("enable_filament_dynamic_map", new ConfigOptionBool(false));

    // NOTE: has_filament_switcher is deliberately NOT forced to false. It reflects a
    // connected AMS; forcing it off collapses multi-filament (T1+) slicing to a single
    // tool. The value stored in the 3mf project settings is used as-is.

    Print print;
    auto st = print.apply(model, print_config);
    if (st == PrintBase::APPLY_STATUS_INVALIDATED) {
        boost::nowide::cerr << "error: print.apply invalidated the print state\n";
        return 3;
    }

    print.process();

    GCodeProcessorResult result;
    std::string outfile = print.export_gcode(output, &result, nullptr);

    if (outfile.empty()) {
        boost::nowide::cerr << "error: export_gcode produced no output\n";
        for (const GCodeProcessorResult::SliceWarning& w : result.warnings)
            boost::nowide::cerr << "  warning: " << w.msg << "\n";
        return 4;
    }

    boost::nowide::cout << "wrote " << outfile << "\n";
    return 0;
}
