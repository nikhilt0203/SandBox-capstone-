
1. Inherit from Module base class
2. use m_Audio to add AudioStreams and map inputs/outputs
3. Optionally use interfaces in module_interfaces.hpp

To make buildable:
  1. Use MODULE_TYPE_INFO macro in public section of class
  2. Add typename in ModuleTypes and ModuleBank to make it buildable (module_types.hpp)
  3. Add MODULE_TYPE(n) in builder_interface.cpp
  4. add RELEASE_MODULE(typename) (module_builder.hpp destroy())