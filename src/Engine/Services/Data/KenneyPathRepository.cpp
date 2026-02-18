#include "Engine/Services/Data/KenneyPathRepository.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/ILoggerService.h"
#include <iostream>

void KenneyPathRepository::OnInitialize()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    if(log) log->Log("KenneyPathRepository::OnInitialize called.");

    std::string base = "assets/textures/kenney_retro-textures-1/PNG/";

    m_paths[KenneyIDs::Door_Metal_Frame] = base + "door_metal_frame.png";
    m_paths[KenneyIDs::Door_Metal_Gate] = base + "door_metal_gate.png";
    m_paths[KenneyIDs::Door_Metal_Gate_Lock] = base + "door_metal_gate_lock.png";
    m_paths[KenneyIDs::Door_Wood] = base + "door_wood.png";
    m_paths[KenneyIDs::Door_Wood_Frame] = base + "door_wood_frame.png";
    m_paths[KenneyIDs::Door_Wood_Handle] = base + "door_wood_handle.png";
    m_paths[KenneyIDs::Door_Wood_Window] = base + "door_wood_window.png";
    m_paths[KenneyIDs::Door_Wood_Window_Lit] = base + "door_wood_window_lit.png";
    m_paths[KenneyIDs::Floor_Ground_Dirt] = base + "floor_ground_dirt.png";
    m_paths[KenneyIDs::Floor_Ground_Grass] = base + "floor_ground_grass.png";
    m_paths[KenneyIDs::Floor_Ground_Grass_Overlay] = base + "floor_ground_grass_overlay.png";
    m_paths[KenneyIDs::Floor_Ground_Sand] = base + "floor_ground_sand.png";
    m_paths[KenneyIDs::Floor_Ground_Water] = base + "floor_ground_water.png";
    m_paths[KenneyIDs::Floor_Ground_Water_Green] = base + "floor_ground_water_green.png";
    m_paths[KenneyIDs::Floor_Stone] = base + "floor_stone.png";
    m_paths[KenneyIDs::Floor_Stone_Grate] = base + "floor_stone_grate.png";
    m_paths[KenneyIDs::Floor_Stone_Pattern] = base + "floor_stone_pattern.png";
    m_paths[KenneyIDs::Floor_Stone_Pattern_Depth] = base + "floor_stone_pattern_depth.png";
    m_paths[KenneyIDs::Floor_Stone_Pattern_Small] = base + "floor_stone_pattern_small.png";
    m_paths[KenneyIDs::Floor_Stone_Pattern_Small_Depth] = base + "floor_stone_pattern_small_depth.png";
    m_paths[KenneyIDs::Floor_Stone_Sand_Grate] = base + "floor_stone_sand_grate.png";
    m_paths[KenneyIDs::Floor_Stone_Sand_Inset] = base + "floor_stone_sand_inset.png";
    m_paths[KenneyIDs::Floor_Stone_Sand_Random] = base + "floor_stone_sand_random.png";
    m_paths[KenneyIDs::Floor_Stone_Sand_Random_Depth] = base + "floor_stone_sand_random_depth.png";
    m_paths[KenneyIDs::Floor_Stone_Sand_Trimsheet] = base + "floor_stone_sand_trimsheet.png";
    m_paths[KenneyIDs::Floor_Stone_Trimsheet] = base + "floor_stone_trimsheet.png";
    m_paths[KenneyIDs::Floor_Tiles_Blue_Large] = base + "floor_tiles_blue_large.png";
    m_paths[KenneyIDs::Floor_Tiles_Blue_Small] = base + "floor_tiles_blue_small.png";
    m_paths[KenneyIDs::Floor_Tiles_Blue_Small_Damaged] = base + "floor_tiles_blue_small_damaged.png";
    m_paths[KenneyIDs::Floor_Tiles_Sand_Large] = base + "floor_tiles_sand_large.png";
    m_paths[KenneyIDs::Floor_Tiles_Sand_Small] = base + "floor_tiles_sand_small.png";
    m_paths[KenneyIDs::Floor_Tiles_Sand_Small_Damaged] = base + "floor_tiles_sand_small_damaged.png";
    m_paths[KenneyIDs::Floor_Tiles_Tan_Large] = base + "floor_tiles_tan_large.png";
    m_paths[KenneyIDs::Floor_Tiles_Tan_Small] = base + "floor_tiles_tan_small.png";
    m_paths[KenneyIDs::Floor_Tiles_Tan_Small_Damaged] = base + "floor_tiles_tan_small_damaged.png";
    m_paths[KenneyIDs::Floor_Wood_Planks] = base + "floor_wood_planks.png";
    m_paths[KenneyIDs::Floor_Wood_Planks_Damaged] = base + "floor_wood_planks_damaged.png";
    m_paths[KenneyIDs::Floor_Wood_Planks_Depth] = base + "floor_wood_planks_depth.png";
    m_paths[KenneyIDs::Floor_Wood_Planks_Wide] = base + "floor_wood_planks_wide.png";
    m_paths[KenneyIDs::Floor_Wood_Planks_Wide_Damaged] = base + "floor_wood_planks_wide_damaged.png";
    m_paths[KenneyIDs::Floor_Wood_Planks_Wide_Depth] = base + "floor_wood_planks_wide_depth.png";
    m_paths[KenneyIDs::Roof_Clay_Grey_Bottom] = base + "roof_clay_grey_bottom.png";
    m_paths[KenneyIDs::Roof_Clay_Grey_Center] = base + "roof_clay_grey_center.png";
    m_paths[KenneyIDs::Roof_Clay_Grey_Top] = base + "roof_clay_grey_top.png";
    m_paths[KenneyIDs::Roof_Clay_Red_Bottom] = base + "roof_clay_red_bottom.png";
    m_paths[KenneyIDs::Roof_Clay_Red_Center] = base + "roof_clay_red_center.png";
    m_paths[KenneyIDs::Roof_Clay_Red_Top] = base + "roof_clay_red_top.png";
    m_paths[KenneyIDs::Roof_Thatch_Bottom] = base + "roof_thatch_bottom.png";
    m_paths[KenneyIDs::Roof_Thatch_Center] = base + "roof_thatch_center.png";
    m_paths[KenneyIDs::Roof_Thatch_Top] = base + "roof_thatch_top.png";
    m_paths[KenneyIDs::Timber_Square_Clay] = base + "timber_square_clay.png";
    m_paths[KenneyIDs::Timber_Square_Clay_Diagonal] = base + "timber_square_clay_diagonal.png";
    m_paths[KenneyIDs::Timber_Square_Frame] = base + "timber_square_frame.png";
    m_paths[KenneyIDs::Timber_Square_Frame_Diagonal] = base + "timber_square_frame_diagonal.png";
    m_paths[KenneyIDs::Timber_Square_Planks] = base + "timber_square_planks.png";
    m_paths[KenneyIDs::Timber_Square_Planks_Boarded] = base + "timber_square_planks_boarded.png";
    m_paths[KenneyIDs::Timber_Square_Planks_Cross] = base + "timber_square_planks_cross.png";
    m_paths[KenneyIDs::Timber_Square_Planks_Diagonal] = base + "timber_square_planks_diagonal.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Both] = base + "wall_brick_sand_both.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Center] = base + "wall_brick_sand_center.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Center_Banner] = base + "wall_brick_sand_center_banner.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Center_Depth] = base + "wall_brick_sand_center_depth.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Left] = base + "wall_brick_sand_left.png";
    m_paths[KenneyIDs::Wall_Brick_Sand_Right] = base + "wall_brick_sand_right.png";
    m_paths[KenneyIDs::Wall_Brick_Small_Sand] = base + "wall_brick_small_sand.png";
    m_paths[KenneyIDs::Wall_Brick_Small_Sand_Depth] = base + "wall_brick_small_sand_depth.png";
    m_paths[KenneyIDs::Wall_Brick_Small_Stone] = base + "wall_brick_small_stone.png";
    m_paths[KenneyIDs::Wall_Brick_Small_Stone_Depth] = base + "wall_brick_small_stone_depth.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Both] = base + "wall_brick_stone_both.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Center] = base + "wall_brick_stone_center.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Center_Banner] = base + "wall_brick_stone_center_banner.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Center_Depth] = base + "wall_brick_stone_center_depth.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Left] = base + "wall_brick_stone_left.png";
    m_paths[KenneyIDs::Wall_Brick_Stone_Right] = base + "wall_brick_stone_right.png";
    m_paths[KenneyIDs::Wall_Rock] = base + "wall_rock.png";
    m_paths[KenneyIDs::Wall_Rock_Structure] = base + "wall_rock_structure.png";
    m_paths[KenneyIDs::Wall_Stone] = base + "wall_stone.png";
    m_paths[KenneyIDs::Wall_Stone_Depth] = base + "wall_stone_depth.png";
    m_paths[KenneyIDs::Wall_Timber] = base + "wall_timber.png";
    m_paths[KenneyIDs::Wall_Timber_Structure] = base + "wall_timber_structure.png";
    m_paths[KenneyIDs::Wall_Timber_Structure_Cross] = base + "wall_timber_structure_cross.png";
    m_paths[KenneyIDs::Wall_Timber_Structure_Diagonal] = base + "wall_timber_structure_diagonal.png";
    m_paths[KenneyIDs::Wall_Timber_Structure_Horizontal] = base + "wall_timber_structure_horizontal.png";
    m_paths[KenneyIDs::Wall_Timber_Structure_Vertical] = base + "wall_timber_structure_vertical.png";
    m_paths[KenneyIDs::Wall_Wood_Trimsheet] = base + "wall_wood_trimsheet.png";
    m_paths[KenneyIDs::Window_Round_Divided] = base + "window_round_divided.png";
    m_paths[KenneyIDs::Window_Round_Divided_Boarded] = base + "window_round_divided_boarded.png";
    m_paths[KenneyIDs::Window_Round_Divided_Lit] = base + "window_round_divided_lit.png";
    m_paths[KenneyIDs::Window_Round_Pane] = base + "window_round_pane.png";
    m_paths[KenneyIDs::Window_Round_Pane_Boarded] = base + "window_round_pane_boarded.png";
    m_paths[KenneyIDs::Window_Round_Pane_Lit] = base + "window_round_pane_lit.png";
    m_paths[KenneyIDs::Window_Square_Closed] = base + "window_square_closed.png";
    m_paths[KenneyIDs::Window_Square_Divided] = base + "window_square_divided.png";
    m_paths[KenneyIDs::Window_Square_Divided_Boarded] = base + "window_square_divided_boarded.png";
    m_paths[KenneyIDs::Window_Square_Divided_Lit] = base + "window_square_divided_lit.png";
    m_paths[KenneyIDs::Window_Square_Frame] = base + "window_square_frame.png";
    m_paths[KenneyIDs::Window_Square_Horizontal] = base + "window_square_horizontal.png";
    m_paths[KenneyIDs::Window_Square_Horizontal_Boarded] = base + "window_square_horizontal_boarded.png";
    m_paths[KenneyIDs::Window_Square_Horizontal_Lit] = base + "window_square_horizontal_lit.png";
    m_paths[KenneyIDs::Window_Square_Metal] = base + "window_square_metal.png";
    m_paths[KenneyIDs::Window_Square_Metal_Fortified] = base + "window_square_metal_fortified.png";
    m_paths[KenneyIDs::Window_Square_Pane] = base + "window_square_pane.png";
    m_paths[KenneyIDs::Window_Square_Pane_Boarded] = base + "window_square_pane_boarded.png";
    m_paths[KenneyIDs::Window_Square_Pane_Lit] = base + "window_square_pane_lit.png";
    m_paths[KenneyIDs::Window_Square_Vertical] = base + "window_square_vertical.png";
    m_paths[KenneyIDs::Window_Square_Vertical_Boarded] = base + "window_square_vertical_boarded.png";
    m_paths[KenneyIDs::Window_Square_Vertical_Lit] = base + "window_square_vertical_lit.png";
    m_paths[KenneyIDs::Window_Tall_Divided] = base + "window_tall_divided.png";
    m_paths[KenneyIDs::Window_Tall_Divided_Damaged] = base + "window_tall_divided_damaged.png";
    m_paths[KenneyIDs::Window_Tall_Divided_Lit] = base + "window_tall_divided_lit.png";
    m_paths[KenneyIDs::Window_Tall_Pane] = base + "window_tall_pane.png";
    m_paths[KenneyIDs::Window_Tall_Pane_Lit] = base + "window_tall_pane_lit.png";
    m_paths[KenneyIDs::Window_Tall_Rounded] = base + "window_tall_rounded.png";
    m_paths[KenneyIDs::Window_Tall_Rounded_Damaged] = base + "window_tall_rounded_damaged.png";
    m_paths[KenneyIDs::Window_Tall_Rounded_Lit] = base + "window_tall_rounded_lit.png";
    m_paths[KenneyIDs::Window_Tall_Vertical] = base + "window_tall_vertical.png";
    m_paths[KenneyIDs::Window_Tall_Vertical_Lit] = base + "window_tall_vertical_lit.png";
    
    if(log) log->Log("KenneyPathRepository initialized with " + std::to_string(m_paths.size()) + " paths.");
}

std::string KenneyPathRepository::GetPath(int assetID)
{
    // Check if ID exists
    auto id = static_cast<KenneyIDs>(assetID);
    if (m_paths.find(id) != m_paths.end())
    {
        return m_paths[id];
    }
    return "";
}
