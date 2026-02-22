ADDING NEW MODULES:

Modules must inherit from the Module base class, specifying the number of inputs and outputs.
Also need to add the type name to ModuleTypes in builder_interface.hpp
use macro for it to appear in the bank

Optional interfaces to add behavior:

```cpp
class Displayable
{
public:
  virtual ~Displayable() = default;
  
  virtual std::uint32_t color() const { return 0xFFFFFF; };
  virtual std::string_view name() const { return "unnamed"; }
  virtual std::string_view controlName(std::size_t index) const { return "?"; }
  virtual std::string_view inputName(std::size_t index) const { return "in"; };
  virtual std::string_view outputName(std::size_t index) const { return "out"; };
};
```
Override any of these functions to provide custom display name, labels, and color


```cpp
class Controllable
{
public:
  virtual ~Controllable() = default;

  virtual void changeControl(std::size_t index, int delta) = 0;
  virtual std::size_t numControls() = 0;
};
```
Index corresponds to the knob #, delta is +/- 1 per encoder click. Must specify how many controls
your module has by overriding numControls. 
If you use a Controls object to store the control change functions, then you can simply use
"obj.change(index, delta);" in the body of changeControl


```cpp
class Pressable
{
public:
  virtual ~Pressable() = default;

  virtual void onRisingEdge() = 0;
  virtual void onFallingEdge() = 0;
};
```
Override to customize what your module should do on press and on release.

