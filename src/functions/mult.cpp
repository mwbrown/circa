// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../builtins.h"
#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace mult_function {

    CA_FUNCTION(evaluate_f)
    {
        set_float(OUTPUT, FLOAT_INPUT(0) * FLOAT_INPUT(1));
    }

    CA_FUNCTION(evaluate_i)
    {
        set_int(OUTPUT, INT_INPUT(0) * INT_INPUT(1));
    }

    CA_FUNCTION(feedback_evaluate)
    {
#if 0
OLD_FEEDBACK_IMPL_DISABLED
        Term* target = INPUT_TERM(0);
        float desired = FLOAT_INPUT(1);

        float delta = desired - to_float(target);

        // for each input, send a delta divided by the product of all other inputs
        Branch* outputList = feedback_output(CALLER);
        for (int i=0; i < outputList.length(); i++) {
            Term* output = outputList[i];
            Term* outputTarget = target->input(i);
            float balanced_delta = delta * get_feedback_weight(output);

            // Compute a product of all other inputs
            float divisor = i == 0 ? to_float(target->input(1)) : to_float(target->input(0));

            // If this product is too close to 0 then give up. We can't solve x = a * 0
            if (fabs(divisor) < 0.0001) {
                set_float(output, to_float(outputTarget));
                continue;
            }

            // Otherwise, to solve for x = a * b, tell a that it should be closer to x / b
            set_float(output, to_float(outputTarget) + balanced_delta / divisor);
        }
        #endif
    }

    void setup(Branch* kernel)
    {
        Term* mult_i = import_function(kernel, evaluate_i, "mult_i(int,int) -> int");
        Term* mult_f = import_function(kernel, evaluate_f, "mult_f(number,number) -> number");

        #if 0
        FIXME
        function_t::get_feedback_func(mult_f) = 
            import_function(kernel, feedback_evaluate, "mult_feedback(any, number) -> Branch");
        #endif

        MULT_FUNC = create_overloaded_function(kernel, "mult", TermList(mult_i, mult_f));
    }
}
} // namespace circa
