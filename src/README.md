
Creating a module that can be used with the audio system:

1. Derive from sndbx::Patchable and pass the number of inputs and outputs to its constructor. 
   This example is for a sine wave with an FM input and adjustable volume. 

```cpp
    using namespace sndbx;

    class MyModule : public Patchable {
        AudioSynthWaveformModulated synth_;
        AudioGraph::NodeID synth_id_;
        AudioAmplifier amp_;
        AudioGraph::NodeID amp_id_;
    public:
        MyModule() : Patchable{1, 1} {} //1 input, 1 output
        
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
   synth_id_ and amp_id_ are invalidated after this operation (see below). 
```cpp
    using namespace sndbx;

    void unlink(AudioGraph &graph) override { 
        graph.remove_node(amp_id_);
        graph.remove_node(synth_id_);
    }
    
    /* Note: IDs are now invalid ... but the actual AudioStreams stored
    * in our class are unaffected, so the ids could be reseated (but in the system
    * link() and unlink() are only called once)
    */
```

3. Provide the patching points. This function should assume that port.index is valid.
   The structure of the example module is the following:

    The two patch points are the input of "synth" and the output of "amp," and the synths
    are connected . map() defines the mapping
    between the logical module port and the actual point in the audio graph. The first field in the patch_point is the AudioStream's audio graph id, and the second field is the AudioStream port number that should be used. 
    port.type indicates whether the query is for an input or an output. 
   
```cpp
    using namespace sndbx;

    AudioEndpoint map(ModulePort port) const override {
        // Mapping: Module input  0 -> synth input 0
        //          Module output 0 -> amp output  0
        assert(port.index == 0);
        switch (port.type) {
        case ModulePortType::IN:
            return audio::make_patch_point(synth_id_, 0);
        case ModulePortType::OUT: 
            return audio::make_patch_point(amp_id_, 0); 
        }
    }
```

4. Controls
    ModuleParameter - arithmetic type with a min and max. can either clamp or wrap at bounds
    ModuleControl - links the simple primitive with the module

    General pattern for declaring a module parameter and control:

```cpp
    // could also be a lambda directly in the constructor
    float curve(float val, int amt) {
        return val + amt * 0.25f;
    }

    // provide the parameter's min, max, initial
    ModuleParameter<float> frequency_{0.0, 1000.0, 440.0};

    // create a module control with a function pointer T(T, int) to decide
    // how to change the parameter, and a member function pointer to the 
    // member function that should be called if the parameter value changes.
    constexpr static ModuleControl<MyModule, decltype(frequency_)> 
        frequency_control_{curve, &MyModule::update_frequency};
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


