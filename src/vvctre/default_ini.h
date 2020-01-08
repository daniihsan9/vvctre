// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

namespace DefaultINI {

const char* sdl2_config_file = R"(
[Controls]
# The input devices and parameters for each 3DS native input
# It should be in the format of "engine:[engine_name],[param1]:[value1],[param2]:[value2]..."
# Escape characters $0 (for ':'), $1 (for ',') and $2 (for '$') can be used in values

# for button input, the following engines are available:
#  - "keyboard" (default) for keyboard input. Required parameters:
#      - "code": the code of the key to bind
#  - "sdl" for joystick input using SDL. Required parameters:
#      - "port": the port of the joystick to bind
#      - "guid": the GUID of the joystick to bind
#      - "button" (optional): the index of the button to bind
#      - "hat" (optional): the index of the hat to bind as direction buttons
#      - "axis" (optional): the index of the axis to bind
#      - "direction" (only used for hat): the direction name of the hat to bind. Can be "up", "down", "left" or "right"
#      - "threshold" (only used for axis): a float value in (-1.0, 1.0) which the button is
#          triggered if the axis value crosses
#      - "direction" (only used for axis): "+" means the button is triggered when the axis value
#          is greater than the threshold; "-" means the button is triggered when the axis value
#          is smaller than the threshold
button_a=
button_b=
button_x=
button_y=
button_up=
button_down=
button_left=
button_right=
button_l=
button_r=
button_start=
button_select=
button_debug=
button_gpio14=
button_zl=
button_zr=
button_home=

# for analog input, the following engines are available:
#  - "analog_from_button" (default) for emulating analog input from direction buttons. Required parameters:
#      - "up", "down", "left", "right": sub-devices for each direction.
#          Should be in the format as a button input devices using escape characters, for example, "engine$0keyboard$1code$00"
#      - "modifier": sub-devices as a modifier.
#      - "modifier_scale": a float number representing the applied modifier scale to the analog input.
#          Must be in range of 0.0-1.0. Defaults to 0.5
#  - "sdl" for joystick input using SDL. Required parameters:
#      - "port": the port of the joystick to bind
#      - "guid": the GUID of the joystick to bind
#      - "axis_x": the index of the axis to bind as x-axis (default to 0)
#      - "axis_y": the index of the axis to bind as y-axis (default to 1)
circle_pad=
c_stick=

# for motion input, the following engines are available:
#  - "motion_emu" (default) for emulating motion input from mouse input. Required parameters:
#      - "update_period": update period in milliseconds (default to 100)
#      - "sensitivity": the coefficient converting mouse movement to tilting angle (default to 0.01)
#      - "tilt_clamp": the max value of the tilt angle in degrees (default to 90)
#  - "cemuhookudp" reads motion input from a udp server that uses cemuhook's udp protocol
motion_device=

# for touch input, the following engines are available:
#  - "emu_window" (default) for emulating touch input from mouse input to the emulation window. No parameters required
#  - "cemuhookudp" reads touch input from a udp server that uses cemuhook's udp protocol
#      - "min_x", "min_y", "max_x", "max_y": defines the udp device's touch screen coordinate system
touch_device=

# Most desktop operating systems do not expose a way to poll the motion state of the controllers
# so as a way around it, cemuhook created a udp client/server protocol to broadcast the data directly
# from a controller device to the client program. vvctre has a client that can connect and read
# from any cemuhook compatible motion program.

# IPv4 address of the udp input server (Default "127.0.0.1")
udp_input_address=

# Port of the udp input server. (Default 26760)
udp_input_port=

# The pad to request data on. Should be between 0 (Pad 1) and 3 (Pad 4). (Default 0)
udp_pad_index=

[Core]
# Whether to use the Just-In-Time (JIT) compiler for CPU emulation
# 0: Interpreter (slow), 1 (default): JIT (fast)
use_cpu_jit =

# Change the clock frequency of the emulated 3DS CPU.
# Underclocking can increase the performance of the game at the risk of freezing.
# Overclocking may fix lag that happens on console, but also comes with the risk of freezing.
# Range is any positive integer (but we suspect 25 - 400 is a good idea) Default is 100
cpu_clock_percentage =

# 0 (default): Off, 1: On
use_custom_cpu_ticks =

# Default: 77
custom_cpu_ticks =

# Default: ws://vvctre-multiplayer.glitch.me
multiplayer_url =

[Data Storage]
# Whether to create a virtual SD card.
# 1 (default): Yes, 0: No
use_virtual_sd =

[System]
# The system model that vvctre will try to emulate
# 0: Old 3DS (default), 1: New 3DS
is_new_3ds =

# The system region that vvctre will use during emulation
# -1: Auto-select (default), 0: Japan, 1: USA, 2: Europe, 3: Australia, 4: China, 5: Korea, 6: Taiwan
region_value =

# The clock to use when vvctre starts
# 0: System clock (default), 1: fixed time
init_clock =

# Time used when init_clock is set to fixed_time in the format %Y-%m-%d %H:%M:%S
# set to fixed time. Default 2000-01-01 00:00:01
# Note: 3DS can only handle times later then Jan 1 2000
init_time =

[Renderer]
# Whether to use software or hardware rendering.
# 0: Software, 1 (default): Hardware
use_hw_renderer =

# Whether to use hardware shaders to emulate 3DS shaders
# 0: Software, 1 (default): Hardware
use_hw_shader =

# Reduce stuttering by storing and loading generated shaders to disk
# 0: Off, 1 (default. On)
use_disk_shader_cache =

# Whether to use accurate multiplication in hardware shaders
# 0: Off (Default. Faster, but causes issues in some games) 1: On (Slower, but correct)
shaders_accurate_mul =

# Whether to use the Just-In-Time (JIT) compiler for shader emulation
# 0: Interpreter (slow), 1 (default): JIT (fast)
use_shader_jit =

# Resolution scale factor
# 0: Auto (scales resolution to window size), 1: Native 3DS screen resolution, Otherwise a scale
# factor for the 3DS resolution
resolution_factor =

# Turns on the frame limiter, which will limit frames output to the target game speed
# 0: Off, 1: On (default)
use_frame_limit =

# Limits the speed of the game to run no faster than this value as a percentage of target speed
# 1 - 9999: Speed limit as a percentage of target game speed. 100 (default)
frame_limit =

# The clear color for the renderer. What shows up on the sides of the bottom screen.
# Must be in range of 0.0-1.0. Defaults to 0.0 for all.
bg_red =
bg_blue =
bg_green =

# Whether and how stereoscopic 3D should be rendered
# 0 (default): Off, 1: Side by Side, 2: Anaglyph
render_3d =

# Change 3D intensity
# 0 - 100: intensity. default: 0
factor_3d =

# Whether to enable linear filtering or not
# This is required for some shaders to work correctly
# 0: Nearest, 1 (default): Linear
filter_mode =

# The name of the post processing shader to apply.
# Loaded from shaders if render_3d is off or side by side.
# Loaded from shaders/anaglyph if render_3d is anaglyph
pp_shader_name =

# Forces VSync on the display thread. Usually doesn't impact performance, but on some drivers it can
# so only turn this off if you notice a speed difference.
# 0: Off, 1 (default): On
use_vsync_new =

# 0: Off (default), 1: On
sharper_distant_objects =
ignore_format_reinterpretation =
use_custom_screen_refresh_rate =

# Default: 60.0
custom_screen_refresh_rate =

# Default: 10
min_vertices_per_thread =

[Layout]
# Layout for the screen inside the render window.
# 0 (default): Default Top Bottom Screen, 1: Single Screen Only, 2: Large Screen Small Screen, 3: Side by Side, 4: Large Screen Medium Screen
layout_option =

# Toggle custom layout (using the settings below) on or off.
# 0 (default): Off, 1: On
custom_layout =

# Screen placement when using custom layout option
# 0x, 0y is the top left corner of the render window.
custom_top_left =
custom_top_top =
custom_top_right =
custom_top_bottom =
custom_bottom_left =
custom_bottom_top =
custom_bottom_right =
custom_bottom_bottom =

# Swaps the prominent screen with the other screen.
# For example, if Single Screen is chosen, setting this to 1 will display the bottom screen instead of the top screen.
# 0 (default): Top Screen is prominent, 1: Bottom Screen is prominent
swap_screen =

# Toggle upright orientation, for book style games.
# 0 (default): Off, 1: On
upright_screen =

[Utility]
# Dumps textures as PNG to dump/textures/[Title ID]/.
# 0 (default): Off, 1: On
dump_textures =

# Reads PNG files from load/textures/[Title ID]/ and replaces textures.
# 0 (default): Off, 1: On
custom_textures =

# Loads all custom textures into memory before booting.
# 0 (default): Off, 1: On
preload_textures =

[Audio]
# Whether or not to enable DSP LLE
# 0 (default): No, 1: Yes
enable_dsp_lle =

# Whether or not to run DSP LLE on a different thread
# 0 (default): No, 1: Yes
enable_dsp_lle_thread =

# Which audio output engine to use.
# auto (default): Auto-select, null: No audio output, sdl2: SDL2 (if available)
output_engine =

# Whether or not to enable the audio-stretching post-processing effect.
# This effect adjusts audio speed to match emulation speed and helps prevent audio stutter,
# at the cost of increasing audio latency.
# 0: No, 1 (default): Yes
enable_audio_stretching =

# Which audio device to use.
# auto (default): Auto-select
output_device =

# Output volume.
# 1.0 (default): 100%, 0.0; mute
volume =

# 0 (default): none, 1: real, 2: static
mic_input_type =

mic_input_device =

# Only used in DSP HLE and requires audio stretching to be enabled to work properly.
# Default: 1.0. Must be greater than zero.
audio_speed =

[Camera]
# Which camera engine to use for the right outer camera
# blank (default): a dummy camera that always returns black image
camera_outer_right_name =

# A config string for the right outer camera. Its meaning is defined by the camera engine
camera_outer_right_config =

# The image flip to apply
# 0: None (default), 1: Horizontal, 2: Vertical, 3: Reverse
camera_outer_right_flip =

# ... for the left outer camera
camera_outer_left_name =
camera_outer_left_config =
camera_outer_left_flip =

# ... for the inner camera
camera_inner_name =
camera_inner_config =
camera_inner_flip =

[Miscellaneous]
# A filter which removes logs below a certain logging level.
# Examples: *:Debug Kernel.SVC:Trace Service.*:Critical
log_filter = *:Info

[Debugging]
# Record frame time data, can be found in the log directory. Boolean value
record_frame_times =
# Port for listening to GDB connections.
use_gdbstub=false
gdbstub_port=24689
# To LLE a service module add "LLE\<module name>=true"
)";
}
