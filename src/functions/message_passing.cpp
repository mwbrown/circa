// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "../common_headers.h"

#include "../importing.h"
#include "../importing_macros.h"

namespace circa {
namespace message_passing_function {

    List* get_message_queue(EvalContext* context, const char* name)
    {
        Dict* hub = &context->messages;
        return List::lazyCast(hub->insert(name));
    }

    CA_FUNCTION(evaluate_send)
    {
        TaggedValue* input = INPUT(1);

        List* inboxState = get_message_queue(CONTEXT, STRING_INPUT(0));

        copy(input, inboxState->append());
    }

    CA_FUNCTION(evaluate_receive)
    {
        List* input = get_message_queue(CONTEXT, STRING_INPUT(0));
        copy(input, OUTPUT);
        input->resize(0);
    }

    void setup(Branch* kernel)
    {
        import_function(kernel, evaluate_send, "def send(string inbox_name, any)");
        import_function(kernel, evaluate_receive, "def receive(string inbox_name) -> List");
    }
}
}
