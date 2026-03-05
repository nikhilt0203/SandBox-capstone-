1. inherit from Module base class
2. use m_Audio API to add AudioStreams and map inputs/outputs
3. optionally use interfaces in module_interfaces.hpp
4. add type in ModuleTypes and ModuleBank if it's buildable (module_types.hpp)
5. add builder.make<get<(1 more than previous)>>... to switch statement in builder_interface.hpp
