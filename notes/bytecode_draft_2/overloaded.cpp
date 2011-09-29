    void writeBytecode(Term* term, BytecodeWriter* writer)
    {
        Branch* contents = nested_contents(term);
        if (contents->length() > 0)
            bc_call(writer, contents->get(0));
        else
            bc_write_call_op(writer, term, evaluate_dynamic_overload);
    }

