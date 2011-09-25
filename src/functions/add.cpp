// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../circa.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace add_function {

    CA_FUNCTION(add_i_evaluate)
    {
        int result = 0;
        for (int i=0; i < NUM_INPUTS; i++)
            result += INT_INPUT(i);
        set_int(OUTPUT, result);
    }

    CA_FUNCTION(add_f_evaluate)
    {
        float result = 0.0;
        for (int i=0; i < NUM_INPUTS; i++)
            result += FLOAT_INPUT(i);
        set_float(OUTPUT, result);
    }

    CA_FUNCTION(add_feedback)
    {
        #if 0
        OLD_FEEDBACK_IMPL_DISABLED
        Term* target = INPUT_TERM(0);
        float desired = FLOAT_INPUT(1);

        float delta = desired - to_float(target);

        Branch* outputList = feedback_output(CALLER);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);
            set_float(output, to_float(outputTarget) + balanced_delta);
        }
        #endif
    }

    void setup(Branch* kernel)
    {
        Term* add_i = import_function(kernel, add_i_evaluate, "add_i(int...) -> int");
        Term* add_f = import_function(kernel, add_f_evaluate, "add_f(number...) -> number");

        get_function_attrs(add_f)->feedbackFunc =
            import_function(kernel, add_feedback, "add_feedback(any, number)");

        ADD_FUNC = create_overloaded_function(kernel, "add", TermList(add_i, add_f));
    }
} // namespace add_function
} // namespace circa
