# Blender FLIP Fluids Add-on
# Copyright (C) 2025 Ryan L. Guy & Dennis Fassbaender
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import bpy, os
from bpy.props import (
        BoolProperty,
        EnumProperty,
        FloatProperty,
        IntProperty,
        PointerProperty,
        StringProperty
        )

from .custom_properties import NewMinMaxIntProperty
from .. import types
from ..utils import version_compatibility_utils as vcu


class DomainAdvancedProperties(bpy.types.PropertyGroup):
    conv = vcu.convert_attribute_to_28
    
    min_max_time_steps_per_frame = NewMinMaxIntProperty(
            name_min="Min Substeps",
            description_min="Minimum number of substeps per frame calculation",
            min_min=1, max_min=1000000,
            soft_max_min=100,
            default_min=1,

            name_max="Max Substeps",
            description_max="Maximum number of substeps per frame calculation",
            min_max=1, max_max=1000000,
            soft_max_max=100,
            default_max=24,
            ); exec(conv("min_max_time_steps_per_frame"))
    enable_adaptive_obstacle_time_stepping = BoolProperty(
            name="Enable Adaptive Time Stepping for Obstacles",
            description="Include obstacle velocities when calculating number"
                " of frame substeps. Enabling may improve the accuracy of"
                " fluid-solid interaction for fast moving obstacles, but"
                " may take longer to simulate",
            default = False,
            ); exec(conv("enable_adaptive_obstacle_time_stepping"))
    enable_adaptive_force_field_time_stepping = BoolProperty(
            name="Enable Adaptive Time Stepping for Force Fields",
            description="Include force field velocities when calculating number"
                " of frame substeps. Enabling may improve the accuracy of"
                " fluid-forcefield interaction for fast moving force fields, but"
                " will take longer to simulate",
            default = False,
            ); exec(conv("enable_adaptive_force_field_time_stepping"))
    particle_jitter_factor = FloatProperty(
            name="Particle Jitter",
            description="Amount of random jitter that is added to newly spawned"
                " fluid particles. Higher values may improve simulation accuracy",
            min=0.0, max=1.0,
            default=1.0,
            precision=2,
            subtype='FACTOR',
            ); exec(conv("particle_jitter_factor"))
    jitter_surface_particles = BoolProperty(
            name="Jitter Surface Particles",
            description="If disabled, a random jitter position will only be applied to particles within"
            " the interior of the Inflow/Fluid object shape. If enabled, all emitted particles"
            " will be jittered. Enabling may cause bumpy mesh artifacts on the"
            " initial surface mesh shape and may require additional smoothing."
            " Enabling is recommended for fluid particle effects and results in"
            " more natural particle generation",
            default=False,
            ); exec(conv("jitter_surface_particles"))
    pressure_solver_max_iterations = IntProperty(
            name="Pressure Solver Max Iterations",
            description="Maximum number of iterations that the pressure solver is allowed"
                " to run during a substep. The default value of 900 is often a good choice and does"
                " not need to be changed. See documentation for more information on this setting",
            min=1, soft_max=10000,
            default=900,
            ); exec(conv("pressure_solver_max_iterations"))
    pressure_solver_method = EnumProperty(
            name="Pressure Solver Method",
            description="Linear solver backend for pressure projection",
            items=types.pressure_solver_methods,
            default='PRESSURE_SOLVER_METHOD_PCG',
            options={'HIDDEN'},
            ); exec(conv("pressure_solver_method"))
    viscosity_solver_max_iterations = IntProperty(
            name="Viscosity Solver Max Iterations",
            description="Maximum number of iterations that the viscosity solver is allowed"
                " to run during a substep. The default value of 900 is often a good choice and does"
                " not need to be changed. See documentation for more information on this setting",
            min=1, soft_max=10000,
            default=900,
            ); exec(conv("viscosity_solver_max_iterations"))
    velocity_transfer_method = EnumProperty(
            name="Velocity Transfer Method",
            description="Simulation method to use",
            items=types.velocity_transfer_methods,
            default='VELOCITY_TRANSFER_METHOD_FLIP',
            options={'HIDDEN'},
            ); exec(conv("velocity_transfer_method"))
    PICFLIP_ratio = FloatProperty(
            name="PIC/FLIP Ratio",
            description="Ratio of PIC velocity to FLIP velocity update mixture."
                " PIC velocity method is not very accurate, but stable. FLIP"
                " velocity method is very accurate, but less stable. Using a"
                " value of 0.0 results in a completely FLIP simulator, while"
                " using a value of 1.0 results in a completely PIC simulator",
            min=0.0, max=1.0,
            default=0.05,
            precision=2,
            subtype='FACTOR',
            ); exec(conv("PICFLIP_ratio"))
    PICAPIC_ratio = FloatProperty(
            name="PIC/APIC Ratio",
            description="Placeholder",
            min=0.0, max=1.0,
            default=0.00,
            precision=2,
            subtype='FACTOR',
            ); exec(conv("PICAPIC_ratio"))
    CFL_condition_number = IntProperty(
            name="Safety Factor (CFL Number)",
            description="Maximum number of voxels that a particle can travel"
                " in a single substep. A larger number may speed up simulation"
                " baking by reducing the number of required substeps at the"
                " cost of simulation accuracy",
            min=1, max=30,
            default=5,
            ); exec(conv("CFL_condition_number"))
    enable_extreme_velocity_removal = BoolProperty(
            name="Remove particles with extreme velocities",
            description="Attempt to remove extreme particle velocities that"
                " cause the simulator to exceed the maximum number of allowed"
                " frame substeps. Enabling this option may prevent simulation"
                " blow-up in extreme cases. It is not recommended to disable"
                " this option outside of experimentation and testing. Disabling"
                " can result in unstable simulations and/or extreme simulation times",
            default=True,
            ); exec(conv("enable_extreme_velocity_removal"))
    enable_gpu_features = BoolProperty(
            name="Enable GPU Features",
            description="Enable simulator to accelerate some computations"
                " with your GPU device. TIP: Compare simulation performance"
                " with this setting on/off to test what is faster on your"
                " hardware setup. Note: you may only notice a difference on"
                " higher resolution simulations",
            default=True
            ); exec(conv("enable_gpu_features"))
    num_threads_auto_detect = IntProperty(
            name="Threads",
            description="Number of threads to use simultaneously while simulating",
            min=1, max=1024,
            default=1,
            ); exec(conv("num_threads_auto_detect"))
    num_threads_fixed = IntProperty(
            name="Threads",
            description="Number of threads to use simultaneously while simulating",
            min=1, max=1024,
            default=4,
            ); exec(conv("num_threads_fixed"))
    threading_mode = EnumProperty(
            name="Threading Mode",
            description="Determing the amount of simulation threads used",
            items=types.threading_modes,
            default='THREADING_MODE_AUTO_DETECT',
            update=lambda self, context: self.initialize_num_threads_auto_detect(),
            options={'HIDDEN'},
            ); exec(conv("threading_mode"))
    enable_adaptive_phase_field_level_set = BoolProperty(
            name="Enable Adaptive Phase Field / MSBG",
            description="Enable experimental adaptive phase-field level set reconstruction with an MSBG sparse grid. Improves thin and fast surface detail, but increases memory and solve cost",
            default=False,
            options={'HIDDEN'},
            ); exec(conv("enable_adaptive_phase_field_level_set"))
    adaptive_phase_field_sparse_block_size = IntProperty(
            name="Adaptive Phase Field Block Size",
            description="Sparse grid block width in cells. Smaller blocks adapt more locally but add overhead; larger blocks are faster but less adaptive",
            min=2, max=64,
            default=16,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_sparse_block_size"))
    adaptive_phase_field_levels = IntProperty(
            name="Adaptive Phase Field Levels",
            description="Number of adaptive hierarchy levels. More levels capture fine detail better, but can increase solver complexity and runtime",
            min=1, max=8,
            default=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_levels"))
    adaptive_phase_field_far_distance = FloatProperty(
            name="Adaptive Phase Field Far Distance",
            description="Half-width of the active narrow band around the interface in voxels. Larger values improve stability and continuity but are slower",
            min=0.1, max=64.0,
            default=3.0,
            precision=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_far_distance"))
    adaptive_phase_field_smoothing_iterations = IntProperty(
            name="Adaptive Phase Field Smoothing Iterations",
            description="Number of smoothing passes on the phase field each update. More passes reduce noise, but too many can blur or inflate the surface",
            min=0, max=100,
            default=5,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_smoothing_iterations"))
    adaptive_phase_field_smoothing_time_step = FloatProperty(
            name="Adaptive Phase Field Smoothing Time Step",
            description="Strength of each smoothing pass. Higher values smooth faster but can over-diffuse detail; lower values preserve sharper features",
            min=0.001, max=1.0,
            default=0.05,
            precision=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_smoothing_time_step"))
    adaptive_phase_field_smoothing_band_layers = IntProperty(
            name="Adaptive Phase Field Smoothing Band Layers",
            description="Extra narrow-band layers included in smoothing. More layers improve robustness around fast motion, fewer layers keep interfaces sharper",
            min=0, max=16,
            default=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_smoothing_band_layers"))
    adaptive_phase_field_velocity_refinement_scale = FloatProperty(
            name="Adaptive Phase Field Velocity Refinement Scale",
            description="How strongly high velocity regions force finer reconstruction. Higher values preserve splashes and thin jets better, but add cost and possible noise",
            min=0.1, max=64.0,
            default=4.0,
            precision=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_velocity_refinement_scale"))
    adaptive_phase_field_velocity_band_expansion_scale = FloatProperty(
            name="Adaptive Phase Field Velocity Band Expansion Scale",
            description="How strongly fast motion expands local support/band coverage. Higher values reduce under-sampling in fast regions but can widen the interface",
            min=0.1, max=16.0,
            default=1.5,
            precision=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_velocity_band_expansion_scale"))
    adaptive_phase_field_alpha_phi = FloatProperty(
            name="Adaptive Phase Field Alpha Phi",
            description="Compression factor for mapping accumulated phase density to phase value (alpha_phi). Higher values sharpen transitions; lower values soften/blend",
            min=0.01, max=8.0,
            default=1.0,
            precision=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_alpha_phi"))
    adaptive_phase_field_density_threshold = FloatProperty(
            name="Adaptive Phase Field Density Threshold",
            description="Minimum accumulated phase density before activation (rho_min). Higher values suppress low-density noise, but can remove very thin fluid",
            min=0.0, max=16.0,
            default=0.0,
            precision=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_density_threshold"))
    adaptive_phase_field_variable_density_pressure_projection = BoolProperty(
            name="Enable Variable-Density Pressure Projection",
            description="Use phase-field-driven variable-density pressure coefficients (beta = 1/rho). Improves liquid-air coupling near the interface, but can be harder to solve",
            default=False,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_variable_density_pressure_projection"))
    adaptive_phase_field_liquid_density = FloatProperty(
            name="Adaptive Phase Field Liquid Density",
            description="Liquid density used by variable-density pressure projection. Increasing liquid-to-gas contrast strengthens liquid dominance and interface sharpness",
            min=0.001, max=100000.0,
            default=1000.0,
            precision=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_liquid_density"))
    adaptive_phase_field_gas_density = FloatProperty(
            name="Adaptive Phase Field Gas Density",
            description="Gas density used by variable-density pressure projection. Higher values increase gas inertia and reduce density contrast against liquid",
            min=0.001, max=100000.0,
            default=1.0,
            precision=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_gas_density"))
    adaptive_phase_field_pressure_air_band_width = IntProperty(
            name="Adaptive Phase Field Pressure Air Band Width",
            description="Number of air cells outside the surface included in variable-density pressure solve. Larger bands improve coupling/stability but increase solve time",
            min=0, max=64,
            default=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_pressure_air_band_width"))
    adaptive_phase_field_particle_adaptivity = BoolProperty(
            name="Enable Adaptive Phase Field Particle Adaptivity",
            description="Enable marker particle split/coarsen adaptivity from phase-field distance. Keeps detail near the interface while reducing particle count elsewhere",
            default=False,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_particle_adaptivity"))
    adaptive_phase_field_particle_max_level = IntProperty(
            name="Adaptive Phase Field Particle Max Level",
            description="Maximum coarsening level for adaptive particles away from detail regions. Higher values are faster and lighter, but can lose detail accuracy",
            min=0, max=8,
            default=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_particle_max_level"))
    adaptive_phase_field_particle_coarsen_delay = IntProperty(
            name="Adaptive Phase Field Particle Coarsen Delay",
            description="Number of adaptivity passes to wait before coarsening. Higher values reduce split/coarsen flicker but keep more particles longer",
            min=0, max=64,
            default=3,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_particle_coarsen_delay"))
    adaptive_phase_field_particle_min_particles_per_cell = IntProperty(
            name="Adaptive Phase Field Particle Min Particles Per Cell",
            description="Minimum marker particles kept per cell during coarsening. Higher values improve stability and transfer quality, but reduce performance gains",
            min=1, max=128,
            default=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_particle_min_particles_per_cell"))
    adaptive_phase_field_two_phase_particles = BoolProperty(
            name="Enable Adaptive Phase Field Two-Phase Particles",
            description="Enable explicit liquid and air marker particles with phase/mass-aware transfer. Improves two-phase behavior, but increases memory and runtime",
            default=False,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_two_phase_particles"))
    adaptive_phase_field_air_particle_band_width = IntProperty(
            name="Adaptive Phase Field Air Particle Band Width",
            description="Air particle maintenance band width outside the interface in cells. Larger values improve two-phase coupling robustness, but add particles",
            min=0, max=64,
            default=2,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_air_particle_band_width"))
    adaptive_phase_field_air_particles_per_cell = IntProperty(
            name="Adaptive Phase Field Air Particles Per Cell",
            description="Target air particles per interface-band cell. Higher values reduce transfer noise and stabilize two-phase coupling, but are slower",
            min=1, max=64,
            default=1,
            options={'HIDDEN'},
            ); exec(conv("adaptive_phase_field_air_particles_per_cell"))
    enable_fracture_optimization = BoolProperty(
            name="Enable Fracture Optimizations",
            description="Enable optimizations when using animated fracture simulations as"
                " FLIP obstacles. These optimizations can greatly improve simulation performance"
                " when there are a large number of small separate FLIP Obstacle objects, such"
                " as in Cell Fracture and RBDLab simulations. Not recommended for fracture simulations"
                " where all pieces are contained within a single FLIP Obstacle as this can harm performance. Enabling"
                " this option can increase memory requirements",
            default = False,
            options={'HIDDEN'},
            ); exec(conv("enable_fracture_optimization"))
    enable_asynchronous_meshing = BoolProperty(
            name="Enable Async Meshing",
            description="Run mesh generation process in a separate thread while"
                " the simulation is running. May increase simulation performance"
                " but will use more RAM if enabled",
            default = True,
            ); exec(conv("enable_asynchronous_meshing"))
    enable_volume_preserving_surface_smoothing = BoolProperty(
            name="Volume-Preserving Surface Smoothing",
            description="Use a two-pass smoothing scheme for the reconstructed fluid surface to reduce volume loss"
                " while still removing high-frequency meshing noise",
            default=False,
            ); exec(conv("enable_volume_preserving_surface_smoothing"))
    precompute_static_obstacles = BoolProperty(
            name="Precompute Static Obstacles",
            description="Precompute data for static obstacles. If enabled,"
                " the simulator will avoid recomputing data for non-animated"
                " obstacles. Increases simulation performance but will use"
                " more RAM if enabled",
            default = True,
            ); exec(conv("precompute_static_obstacles"))
    reserve_temporary_grids = BoolProperty(
            name="Reserve Temporary Grid Memory",
            description="Reserve space in memory for temporary grids. Increases"
                " simulation performance for scenes with animated or keyframed"
                " obstacles but will use more RAM if enabled",
            default = True,
            ); exec(conv("reserve_temporary_grids"))
    disable_changing_topology_warning = BoolProperty(
            name="Disable Changing Topology Warning",
            description="Disable warning that is displayed when exporting an"
            " animated mesh with changing topology. WARNING: mesh velocity"
            " data cannot be computed for meshes that change topology. This may"
            " result in unexpected object-fluid interaction as these objects will"
            " not be able to push around the fluid",
            default=False,
            options={'HIDDEN'},
            ); exec(conv("disable_changing_topology_warning"))

    surface_tension_substeps_exceeded_tooltip = BoolProperty(
            name="Warning: Not Enough Max Substeps", 
            description="The estimated number of Surface Tension substeps per frame exceeds the Max Frame"
                " Substeps value. This can cause an unstable simulation. Either decrease the amount of"
                " Surface Tension in the FLIP Fluid World panel to lower the number of required substeps or"
                " increase the number of allowed Max Frame Substeps in the FLIP Fluid Advanced panel", 
            default=True,
            ); exec(conv("surface_tension_substeps_exceeded_tooltip"))

    frame_substeps_expanded = BoolProperty(default=True); exec(conv("frame_substeps_expanded"))
    simulation_method_expanded = BoolProperty(default=True); exec(conv("simulation_method_expanded"))
    simulation_stability_expanded = BoolProperty(default=False); exec(conv("simulation_stability_expanded"))
    multithreading_expanded = BoolProperty(default=True); exec(conv("multithreading_expanded"))
    warnings_and_errors_expanded = BoolProperty(default=False); exec(conv("warnings_and_errors_expanded"))


    def register_preset_properties(self, registry, path):
        add = registry.add_property
        add(path + ".min_max_time_steps_per_frame",              "Min-Max Time Steps",                 group_id=0)
        add(path + ".enable_adaptive_obstacle_time_stepping",    "Adaptive Obstacle Stepping",         group_id=0)
        add(path + ".enable_adaptive_force_field_time_stepping", "Adaptive Force Field Stepping",      group_id=0)
        add(path + ".particle_jitter_factor",                    "Jitter Factor",                      group_id=0)
        add(path + ".jitter_surface_particles",                  "Jitter Surface Particles",           group_id=0)
        add(path + ".pressure_solver_max_iterations",            "Pressure Solver Iterations",         group_id=0)
        add(path + ".pressure_solver_method",                    "Pressure Solver Method",             group_id=0)
        add(path + ".viscosity_solver_max_iterations",           "Viscosity Solver Iterations",        group_id=0)
        add(path + ".velocity_transfer_method",                  "Velocity Transfer Method",           group_id=0)
        add(path + ".PICFLIP_ratio",                             "PIC/FLIP Ratio",                     group_id=0)
        add(path + ".PICAPIC_ratio",                             "PIC/APIC Ratio",                     group_id=0)
        add(path + ".enable_adaptive_phase_field_level_set",      "Enable Adaptive Phase Field/MSBG",    group_id=0)
        add(path + ".adaptive_phase_field_sparse_block_size",      "Adaptive Phase Field Block Size",     group_id=0)
        add(path + ".adaptive_phase_field_levels",                 "Adaptive Phase Field Levels",         group_id=0)
        add(path + ".adaptive_phase_field_far_distance",           "Adaptive Phase Field Far Distance",   group_id=0)
        add(path + ".adaptive_phase_field_smoothing_iterations",    "Adaptive Phase Field Smoothing Iterations", group_id=0)
        add(path + ".adaptive_phase_field_smoothing_time_step",     "Adaptive Phase Field Smoothing Time Step",  group_id=0)
        add(path + ".adaptive_phase_field_smoothing_band_layers",   "Adaptive Phase Field Smoothing Band Layers", group_id=0)
        add(path + ".adaptive_phase_field_velocity_refinement_scale","Adaptive Phase Field Velocity Refinement Scale", group_id=0)
        add(path + ".adaptive_phase_field_velocity_band_expansion_scale", "Adaptive Phase Field Velocity Band Expansion Scale", group_id=0)
        add(path + ".adaptive_phase_field_alpha_phi",              "Adaptive Phase Field Alpha Phi",      group_id=0)
        add(path + ".adaptive_phase_field_density_threshold",      "Adaptive Phase Field Density Threshold", group_id=0)
        add(path + ".adaptive_phase_field_variable_density_pressure_projection", "Adaptive Phase Field Variable Density Pressure", group_id=0)
        add(path + ".adaptive_phase_field_liquid_density",         "Adaptive Phase Field Liquid Density", group_id=0)
        add(path + ".adaptive_phase_field_gas_density",            "Adaptive Phase Field Gas Density",    group_id=0)
        add(path + ".adaptive_phase_field_pressure_air_band_width","Adaptive Phase Field Pressure Air Band Width", group_id=0)
        add(path + ".adaptive_phase_field_particle_adaptivity",    "Adaptive Phase Field Particle Adaptivity", group_id=0)
        add(path + ".adaptive_phase_field_particle_max_level",     "Adaptive Phase Field Particle Max Level", group_id=0)
        add(path + ".adaptive_phase_field_particle_coarsen_delay", "Adaptive Phase Field Particle Coarsen Delay", group_id=0)
        add(path + ".adaptive_phase_field_particle_min_particles_per_cell", "Adaptive Phase Field Particle Min Particles Per Cell", group_id=0)
        add(path + ".adaptive_phase_field_two_phase_particles",    "Adaptive Phase Field Two-Phase Particles", group_id=0)
        add(path + ".adaptive_phase_field_air_particle_band_width","Adaptive Phase Field Air Particle Band Width", group_id=0)
        add(path + ".adaptive_phase_field_air_particles_per_cell", "Adaptive Phase Field Air Particles Per Cell", group_id=0)
        add(path + ".CFL_condition_number",                      "CFL",                                group_id=0)
        add(path + ".enable_extreme_velocity_removal",           "Enable Extreme Velocity Removal",    group_id=0)
        add(path + ".enable_gpu_features",                       "Enable GPU Features",                group_id=1)
        add(path + ".threading_mode",                            "Threading Mode",                     group_id=1)
        add(path + ".num_threads_fixed",                         "Num Threads (fixed)",                group_id=1)
        add(path + ".enable_asynchronous_meshing",               "Async Meshing",                      group_id=1)
        add(path + ".enable_volume_preserving_surface_smoothing","Volume Preserving Surface Smoothing", group_id=1)
        add(path + ".enable_fracture_optimization",              "Enable Fracture Optimization",        group_id=1)
        add(path + ".precompute_static_obstacles",               "Precompute Static Obstacles",        group_id=1)
        add(path + ".reserve_temporary_grids",                   "Reserve Temporary Grid Memory",      group_id=1)
        add(path + ".disable_changing_topology_warning",         "Disable Changing Topology Warning",  group_id=1)


    def initialize(self):
        self.initialize_num_threads_auto_detect()


    def load_post(self):
        self.initialize_num_threads_auto_detect()
        self._sanitize_legacy_enum_values()
        

    def initialize_num_threads_auto_detect(self):
        original_threads_mode = bpy.context.scene.render.threads_mode
        bpy.context.scene.render.threads_mode = 'AUTO'
        self.num_threads_auto_detect = bpy.context.scene.render.threads
        bpy.context.scene.render.threads_mode = original_threads_mode


    def _sanitize_legacy_enum_values(self):
        # Legacy scenes may store enum backing values instead of enum identifiers.
        # Normalize these values so UI/bake paths do not hit invalid enum warnings.
        velocity_value = self.get("velocity_transfer_method", None)
        velocity_map = {
            1: 'VELOCITY_TRANSFER_METHOD_FLIP',
            "1": 'VELOCITY_TRANSFER_METHOD_FLIP',
            2: 'VELOCITY_TRANSFER_METHOD_APIC',
            "2": 'VELOCITY_TRANSFER_METHOD_APIC',
        }
        normalized_velocity = velocity_map.get(velocity_value)
        if normalized_velocity is not None:
            self.velocity_transfer_method = normalized_velocity

        pressure_value = self.get("pressure_solver_method", None)
        pressure_map = {
            0: 'PRESSURE_SOLVER_METHOD_PCG',
            "0": 'PRESSURE_SOLVER_METHOD_PCG',
            1: 'PRESSURE_SOLVER_METHOD_FPCG',
            "1": 'PRESSURE_SOLVER_METHOD_FPCG',
            2: 'PRESSURE_SOLVER_METHOD_AMG_FPCG',
            "2": 'PRESSURE_SOLVER_METHOD_AMG_FPCG',
        }
        normalized_pressure = pressure_map.get(pressure_value)
        if normalized_pressure is not None:
            self.pressure_solver_method = normalized_pressure


    def _update_min_time_steps_per_frame(self, context):
        if self.min_time_steps_per_frame > self.max_time_steps_per_frame:
            self.max_time_steps_per_frame = self.min_time_steps_per_frame

    def _update_max_time_steps_per_frame(self, context):
        if self.max_time_steps_per_frame < self.min_time_steps_per_frame:
            self.min_time_steps_per_frame = self.max_time_steps_per_frame



def register():
    bpy.utils.register_class(DomainAdvancedProperties)


def unregister():
    bpy.utils.unregister_class(DomainAdvancedProperties)
