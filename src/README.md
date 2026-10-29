
Creating a module that can be used with the audio system:

1. Derive from sndbx::Patchable and pass the number of inputs and outputs to its constructor. 
   This example is for a sine wave with an FM input and adjustable volume. 

```cpp
    using namespace sndbx;

    class MyModule : public Patchable {
        AudioSynthWaveformModulated synth_;
        AudioAmplifier amp_;
        nst::teensy::AudioNodeID synth_id_;
        nst::teensy::AudioNodeID amp_id_;
    public:
        MyModule() : Patchable{1, 1} {}
        
    // class cont. below
```

2. Override link() to connect to the audio graph. In this example, the module internally holds two audio nodes (synth_ and amp_ above) that are connected together. Use the helper add_nodes which automatically cleans up partially formed modules
if any node addition fails. The node IDs are passed as an output parameter.

```cpp
    using namespace sndbx;

    AudioError link(AudioGraph &graph) override {
        const auto error = audio::add_nodes(graph, {&synth_, synth_id_}, {&amp_, amp_id_}); 
        if (error != AudioError::NONE) {
            return error;
        }
        return graph.connect(synth_id_, 0, amp_id_, 0);
    }

    // class continued below...
```

2. Remove all nodes created in link(). The node ids must be stored in order to perform this step. Note that 
   synth_id_ and amp_id_ are invalidated after this operation, but logically if link() were to be called again, 
   the IDs should be correctly reseated.

```cpp
    using namespace sndbx;

    void unlink(AudioGraph &graph) override { 
        graph.remove_node(amp_id_);
        graph.remove_node(synth_id_);
    }
```

3. Provide the patching points. This function should assume that port.index is valid.
   The structure of the example module is the following:

    The two patch points are the input of "synth" and the output of "amp," and the synths
    are connected . map() defines the mapping
    
    between the logical module port and the actual point in the audio graph. 
    The helper function map_port() can be used to separate the logic depending on if the query
    was for an input or an output.
   
```cpp
    using namespace sndbx;

    AudioEndpoint map(ModulePort port) const override {
        assert(port.index == 0);
        // input [0] of MyModule  maps to->  input [0] of internal synth
        // output[0] of MyModule  maps to->  output[0] of internal amplifier

        // Note: the port index argument is ignored, since it
        //       should only ever be 0 given that this module
        //       has 1 input and 1 output port
        auto ins = [this](auto){
            return audio::make_patch_point(synth_id_, 0);
        };
        auto outs = [this](auto){
            return audio::make_patch_point(amp_id_, 0);
        };
        return audio::map_port(port, ins, outs);
    }
```

4. Controls
    Two simple constructs that work together:

    ModuleParameter - simple arithmetic type with a min and max value. Either clamps to [min, max] or wraps around
                      depending on the policy given as a template parameter.
    ModuleControl - holds a curve function that determines how the parameter changes, and a pointer to a member function
                    which is called when the parameter value is modified.

    The curve is just a function pointer T(*)(T,int), so a stateless lambda can also be used. There are several
    generic curve functions provided in the sndbx::curves namespace.

```cpp
    // define the parameter's (min, max, initial) values
    ModuleParameter<float> frequency{0.0, 1000.0, 440.0};

    // define the control, which can be a compile-time constant
    constexpr static auto 
        frequency_ctrl = ModuleControl{curves::exp<float>, &MyModule::update_frequency};
```

    

```cpp
    float MyModule::change_control(ControlChange c) {
        assert(c.index < num_controls());

        switch (c.index) {
        case 0:
            return frequency_control_.change(*this, frequency_, c.amt);
            // ... more cases
        }
        return 0;
    }

```

    When 'frequency_control_.change()' is called, the function 'curve' is called with (frequency_, c.amt) to get the new value. If the value has changed, then the member function to handle the new parameter value is called on the module (passed in as *this).


