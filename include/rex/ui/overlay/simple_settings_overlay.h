/**
 * @file        rex/ui/overlay/simple_settings_overlay.h
 *
 * @brief       Curated user-facing settings overlay.
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <rex/ui/graphics_device_list.h>
#include <rex/ui/imgui_dialog.h>

namespace rex::ui {

void EnsureSimpleSettingsConfig(const std::filesystem::path& config_path);
void SaveSimpleSettingsConfig(const std::filesystem::path& config_path);

struct SimpleProfileInfo {
  std::string id;
  std::string gamertag;
  bool signed_in = true;
};

struct SimpleProfileState {
  std::vector<SimpleProfileInfo> profiles;
  int selected_index = 0;
};

// Raw pad snapshot for overlay navigation (host-side, already merged across
// pads). Poll callback runs on the UI thread every drawn frame.
struct SimpleSettingsGamepad {
  bool connected = false;
  uint16_t buttons = 0;  // X_INPUT_GAMEPAD_* bits
  int16_t thumb_lx = 0;
  int16_t thumb_ly = 0;
};

struct SimpleModInfo {
  std::string id;
  std::string name;
  std::string description;
  std::string version;
  bool enabled = true;
  bool hot_reload = false;
  bool is_player_model = false;
};

struct SimpleAchievementItem {
  uint32_t id = 0;
  std::string name;
  std::string description;
  std::string unachieved_desc;
  std::string category;
  uint32_t gamerscore = 0;
  bool is_secret = false;
  bool unlocked = false;
  std::string unlock_time_str;
  uint32_t progress_current = 0;
  uint32_t progress_total = 0;
  std::vector<std::pair<std::string, bool>> checklist;
};

struct SimpleAchievementData {
  uint32_t total_count = 0;
  uint32_t unlocked_count = 0;
  uint32_t total_gamerscore = 0;
  uint32_t unlocked_gamerscore = 0;
  std::vector<SimpleAchievementItem> items;
};

class SimpleSettingsDialog final : public ImGuiDialog {
 public:
  using LoadProfilesCallback = std::function<SimpleProfileState()>;
  using SaveProfileCallback =
      std::function<void(int selected_index, std::string gamertag, bool signed_in)>;
  using LoadModsCallback = std::function<std::vector<SimpleModInfo>()>;
  using SaveModsCallback = std::function<void(const std::vector<SimpleModInfo>&)>;
  using LoadAchievementsCallback = std::function<SimpleAchievementData()>;
  using ResetAchievementsCallback = std::function<void()>;
  using ResetFilteredAchievementsCallback = std::function<void(const std::vector<uint32_t>&)>;
  using AchievementIconCallback = std::function<void*(uint32_t achievement_id)>;
  using CloseSettingsCallback = std::function<void()>;
  using CloseGameCallback = std::function<void()>;
  using RestartGameCallback = std::function<void()>;
  using PollGamepadCallback = std::function<SimpleSettingsGamepad()>;
  using OpenChallengeEditorCallback = std::function<void()>;

  SimpleSettingsDialog(ImGuiDrawer* drawer, std::filesystem::path config_path,
                       LoadProfilesCallback load_profiles, SaveProfileCallback save_profile,
                       CloseSettingsCallback close_settings, CloseGameCallback close_game,
                       RestartGameCallback restart_game,
                       PollGamepadCallback poll_gamepad = nullptr,
                       LoadModsCallback load_mods = nullptr,
                       SaveModsCallback save_mods = nullptr,
                       LoadAchievementsCallback load_achievements = nullptr);
  ~SimpleSettingsDialog();

  void Show(int initial_category = -1);
  void SelectCategory(int category);
  void Toggle();
  void Hide();
  // One "back" step (Escape / pad B): text edit -> row focus -> category rail
  // -> closed. The Escape keybind routes here so Escape backs out level by
  // level instead of instantly closing.
  void NavigateBack();
  void SetOpenChallengeEditorCallback(OpenChallengeEditorCallback cb) {
    open_challenge_editor_ = std::move(cb);
  }
  void SetLoadAchievementsCallback(LoadAchievementsCallback cb) {
    load_achievements_ = std::move(cb);
  }
  void SetResetAchievementsCallback(ResetAchievementsCallback cb) {
    reset_achievements_ = std::move(cb);
  }
  void SetResetFilteredAchievementsCallback(ResetFilteredAchievementsCallback cb) {
    reset_filtered_achievements_ = std::move(cb);
  }
  void SetAchievementIconCallback(AchievementIconCallback cb) {
    achievement_icon_cb_ = std::move(cb);
  }
  bool visible() const { return visible_; }

 protected:
  void OnDraw(ImGuiIO& io) override;

 private:
  enum class FocusZone { kRail, kContent };

  struct RowSpec;
  struct NavIntents;

  void LoadSettingsFromCvars();
  bool HasSettingsChanges() const;
  void ReloadProfiles();
  void ReloadMods();
  void ReloadAchievements();
  void SaveVideo();
  void SaveProfile();
  void SaveMods();
  bool HasModChanges() const;
  void ApplyAndRestart();

  void BuildRows(std::vector<RowSpec>& rows, int category);
  NavIntents GatherInput(ImGuiIO& io);

  std::filesystem::path config_path_;
  LoadProfilesCallback load_profiles_;
  SaveProfileCallback save_profile_;
  CloseSettingsCallback close_settings_;
  CloseGameCallback close_game_;
  RestartGameCallback restart_game_;
  PollGamepadCallback poll_gamepad_;
  LoadModsCallback load_mods_;
  SaveModsCallback save_mods_;
  LoadAchievementsCallback load_achievements_;
  ResetAchievementsCallback reset_achievements_;
  ResetFilteredAchievementsCallback reset_filtered_achievements_;
  AchievementIconCallback achievement_icon_cb_;
  bool confirm_reset_achievements_ = false;
  bool confirm_reset_filtered_achievements_ = false;
  OpenChallengeEditorCallback open_challenge_editor_;
  SimpleProfileState profiles_;
  std::vector<SimpleModInfo> mods_;
  std::vector<int> staged_mod_states_;
  std::vector<size_t> player_model_mod_indices_;
  int player_model_index_ = 0;
  SimpleAchievementData achievement_data_;
  int achievement_filter_ = 0;
  int achievement_category_filter_ = 0;
  bool visible_ = false;

  // Staged setting values (committed by SaveVideo / SaveProfile).
  GraphicsDeviceList device_list_;
  int graphics_api_index_ = 0;
  int device_index_ = 0;
  int resolution_scale_index_ = 0;
  int frame_cap_index_ = 0;
  int aspect_ratio_index_ = 0;
  int msaa_index_ = 2;
  int shadow_quality_index_ = 2;
  int static_shadow_res_index_ = 2;
  int monitor_index_ = 0;
  int audio_buffer_index_ = 0;
  int language_index_ = 0;
  float field_of_view_ = 60.0f;
  bool fullscreen_ = true;
  bool vsync_ = false;
  bool tearing_ = true;
  bool mnk_mode_ = false;
  bool mnk_capture_mouse_ = false;
  bool profile_signed_in_ = true;
  char gamertag_buf_[32] = {};
  // Live setting values (hot cvars, applied and saved on change).
  bool renderer_native_ = true;
  bool ssao_ = true;
  bool static_shadows_ = true;
  bool shadow_pcss_ = true;
  bool bloom_ = true;
  bool volumetrics_ = true;
  int draw_distance_index_ = 1;
  int stream_probe_index_ = 0;
  bool mode_indicator_ = true;
  bool fps_counter_ = false;
  bool show_coords_ = false;
  bool coord_capture_ = false;
  bool audio_mute_ = false;
  bool rumble_ = true;
  float mnk_sensitivity_ = 1.0f;
  int chord_index_ = 0;
  int input_backend_index_ = 0;
  std::string chord_custom_;
  bool cheat_no_bails_ = false;
  int cheat_drop_height_index_ = 0;
  bool cheat_freeze_timer_ = false;
  int cheat_gravity_index_ = 2;
  int cheat_pop_height_index_ = 0;
  int cheat_offboard_jump_height_index_ = 0;
  int cheat_offboard_jump_speed_index_ = 1;
  int cheat_onboard_speed_index_ = 1;
  int cheat_offboard_speed_index_ = 1;

  // Navigation state.
  FocusZone zone_ = FocusZone::kRail;
  int category_ = 0;
  int rail_sel_ = 0;  // 0..category count-1 = categories; count = the Close Game item
  int row_index_ = 0;
  bool editing_text_ = false;
  bool text_edit_focus_pending_ = false;
  bool pad_active_ = false;      // last nav input came from a pad -> pad legend
  uint16_t prev_pad_buttons_ = 0;
  int held_dir_x_ = 0;           // current held direction (-1/0/1), for repeat
  int held_dir_y_ = 0;
  float repeat_timer_x_ = 0.0f;
  float repeat_timer_y_ = 0.0f;
  float highlight_anim_y_ = -1.0f;  // smoothed selection highlight position
  float rail_anim_y_ = -1.0f;
  float content_scroll_ = 0.0f;       // scroll TARGET, locked to row boundaries
  float content_scroll_anim_ = -1.0f;  // drawn scroll, chases the target
  float wheel_accum_ = 0.0f;           // fractional wheel deltas -> whole notches
  // Swallow the first frame's cursor delta after Show: the pre-open cursor
  // position (or a cursor-mode warp) otherwise reads as mouse motion and
  // steals focus from the rail to whatever row it lands on.
  bool just_shown_ = false;
  float mouse_x_ = -1.0f;        // last mouse position, to detect real motion
  float mouse_y_ = -1.0f;
};

}  // namespace rex::ui
