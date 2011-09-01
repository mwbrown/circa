// Copyright (c) 2007-2010 Paul Hodge. All rights reserved

#include "list_shared.h"

namespace circa {
namespace dynamic_call_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(dynamic_call, "dynamic_call(Function f, List args)")
    {
        Term* function = as_function_pointer(INPUT(0));
        List* inputs = List::checkCast(INPUT(1));

        Term temporaryTerm;
        temporaryTerm.function = function;
        temporaryTerm.type = CALLER->type;

        int numInputs = inputs->length();
        int numOutputs = 1;

        push_stack_frame(CONTEXT, numInputs + numOutputs);

        List* frame = get_stack_frame(CONTEXT, 0);
        temporaryTerm.inputIsns.inputs.resize(numInputs);

        // Populate input instructions, use our stack frame.
        int frameIndex = 0;
        for (int i=0; i < numInputs; i++) {
            copy(inputs->get(i), frame->get(frameIndex));

            InputInstruction* isn = &temporaryTerm.inputIsns.inputs[i];
            isn->type = InputInstruction::LOCAL;
            isn->index = frameIndex;
            isn->relativeFrame = 0;
            frameIndex++;
        }

        temporaryTerm.localsIndex = frameIndex++;

        // Evaluate
        evaluate_single_term(CONTEXT, &temporaryTerm);

        frame = get_stack_frame(CONTEXT, 0);

        // Save the stack frame and pop. (the OUTPUT macro isn't valid until
        // we restore the stack to its original size).
        TaggedValue finishedFrame;
        swap(frame, &finishedFrame);
        pop_stack_frame(CONTEXT);

        swap(list_get_index(&finishedFrame, temporaryTerm.localsIndex), OUTPUT);
    }

    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
