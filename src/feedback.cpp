// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "kernel.h"
#include "subroutine.h"
#include "term.h"
#include "type.h"

#include "feedback.h"

namespace circa {

void handle_feedback_event(EvalContext* context, Term* target, TaggedValue* desired)
{
    // For now, use a simple implementation which cannot handle combining multiple
    // feedback events, or dispatching feedback to multiple sources. All we handle
    // is dispatching across copy/input terms.

    if (target->function == COPY_FUNC) {
        return handle_feedback_event(context, target->input(0), desired);
    } else if (target->function == VALUE_FUNC) {
        bool success = cast(desired, declared_type(target), target);
        if (!success) {
            std::cout << "in handle_feedback, failed to cast "
                << desired->value_type->name << " to "
                << declared_type(target)->name << std::endl;
        }
    } else if (target->function == INPUT_PLACEHOLDER_FUNC) {

#if 0
        FIXME
        ca_assert(context != NULL);

        // Find where we are in context->stack.

        int stackPos;
        for (stackPos = context->callStack.length() - 1; stackPos >= 0; stackPos--) {
            Term* stackTerm = context->callStack[stackPos];
            if (!is_subroutine(stackTerm->function))
                continue;
            if (target->owningBranch == stackTerm->function->nestedContents)
                break;
        }

        ca_assert(stackPos >= 0);

        Term* caller = context->callStack[stackPos];

        ca_assert(caller->function->nestedContents == target->owningBranch);

        int input = get_input_index_of_placeholder(target);

        handle_feedback_event(context, caller->input(input), desired);
#endif
    } else if (target->function == LIST_FUNC) {

        ca_assert(is_list(desired));

        List* desiredList = List::checkCast(desired);
        ca_assert(desiredList->length() == target->numInputs());

        for (int i=0; i < target->numInputs(); i++)
            handle_feedback_event(context, target->input(i), desiredList->get(i));
    } else {
        std::cout << "function doesn't support feedback: " << target->function->name
            << std::endl;
    }
}

#if 0

OLD_FEEDBACK_IMPL_DISABLED

const std::string TRAINING_BRANCH_NAME = "#training";

TermList FeedbackOperation::getFeedback(Term* target, Term* type)
{
    if (!hasPendingFeedback(target))
        return TermList();

    PendingFeedbackList &list = _pending[target];

    // Make a list of values which match this type
    TermList values;

    PendingFeedbackList::const_iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        if (it->type != type)
            continue;

        values.append(it->value);
    }

    return values;
}

void FeedbackOperation::sendFeedback(Term* target, Term* value, Term* type)
{
    FeedbackEntry entry(target,value,type);
    _pending[target].push_back(entry);
}

bool FeedbackOperation::hasPendingFeedback(Term* target)
{
    if (_pending.find(target) == _pending.end())
        return false;

    if (_pending[target].size() == 0)
        return false;

    return true;
}

bool is_trainable(Term* term)
{
    return term->boolPropOptional("trainable", false)
        || term->boolPropOptional("derived-trainable", false);
}

void set_trainable(Term* term, bool value)
{
    term->setBoolProp("trainable", value);
}

void update_derived_trainable_properties(Branch& branch)
{
    for (BranchIterator it(&branch); !it.finished(); it.advance()) {
        // if any of our inputs are trainable then mark us as derived-trainable
        bool found = false;
        for (int i=0; i < it->numInputs(); i++) {
            if (is_trainable(it->input(i))) {
                found = true;
                break;
            }
        }

        it->setBoolProp("derived-trainable", found);
    }
}

void normalize_feedback_branch(Branch& branch)
{
    // Look for any terms that have multiple assign() functions, and combine them with
    // a feedback-accumulator to one assign()
    
    // First, make a map of every assigned-to term and the index of every related 
    // assign() term
    std::map<Term*, std::vector<int> > termToAssignTerms;

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term->function == UNSAFE_ASSIGN_FUNC) {
            Term* target = term->input(0);
            termToAssignTerms[target].push_back(i);
        }
    }

    // Then, iterate over assigned-to terms
    std::map<Term*, std::vector<int> >::const_iterator it;
    for (it = termToAssignTerms.begin(); it != termToAssignTerms.end(); ++it) {
        int assignCount = (int) it->second.size();
        if (assignCount > 1) {

            Term* target = it->first;

            // Remove all of the assign() terms, and make a list of terms to send
            // to the feedback-accumulator
            std::vector<int>::const_iterator index_it;
            TermList accumulatorInputs;

            for (index_it = it->second.begin(); index_it != it->second.end(); ++index_it) {
                int index = *index_it;
                ca_assert(branch[index] != NULL);
                accumulatorInputs.append(branch[index]->input(0));
                branch.set(index, NULL);
            }

            // Create a call to their feedback-accumulator
            // Should probably choose the accumulator func based on type or function
            Term* accumulator = apply(branch, AVERAGE_FUNC, accumulatorInputs);

            // assign() this
            apply(branch, UNSAFE_ASSIGN_FUNC, TermList(accumulator, target));
        }
    }

    branch.removeNulls();
}

void refresh_training_branch(Branch& branch, Branch& trainingBranch)
{
    //#if 0
    update_derived_trainable_properties(branch);
    trainingBranch.clear();

    FeedbackOperation operation;

    // Iterate backwards through the code
    for (BranchIterator it(&branch, true); !it.finished(); ++it) {
        Term* term = *it;

        // Check for feedback(), which does nothing but create a feedback signal
        if (term->function == FEEDBACK_FUNC) {
            Term* target = term->input(0);
            Term* value = term->input(1);
            operation.sendFeedback(target, value, DESIRED_VALUE_FEEDBACK);
            continue;
        }

        // Skip term if it's not trainable
        if (!is_trainable(term))
            continue;

        // Skip if it has no pending feedback
        if (!operation.hasPendingFeedback(term))
            continue;

        Term* feedbackFunc = function_t::get_feedback_func(term->function);

        // Skip term if it has no feedback function
        if (feedbackFunc == NULL) {
            std::cout << "warning: function " << term->function->name
                << " has no feedback function." << std::endl;
            continue;
        }

        // Count the number of trainable inputs
        int numTrainableInputs = 0;
        for (int i=0; i < term->numInputs(); i++)
            if (is_trainable(term->input(i)))
                numTrainableInputs++;

        // Skip this term if it has no numTrainableInputs. As an exception, don't skip functions
        // with 0 inputs. (this includes value())
        if (numTrainableInputs == 0 && (term->numInputs() > 0))
            continue;

        // Apply feedback function
        TermList feedbackTaggedValues = operation.getFeedback(term, DESIRED_VALUE_FEEDBACK);

        // TODO: accumulate desired value
        Term* desiredTaggedValue = feedbackTaggedValues[0];

        if (term->numInputs() == 0) {
            // Just create a feedback term. This is probably an assign() for a value()
            apply(trainingBranch, feedbackFunc, TermList(term, desiredTaggedValue));

        } else if (term->numInputs() == 1) {
            // Create a feedback term with only 1 output
            Term* feedback = apply(trainingBranch, feedbackFunc, TermList(term, desiredTaggedValue));
            operation.sendFeedback(term->input(0), feedback, DESIRED_VALUE_FEEDBACK);

        } else if (term->numInputs() > 1) {

            // If the term has multiple inputs, then the feedback term will have multiple outputs

            // Inputs to feedback func are [originalTerm, desiredTaggedValue]

            Term* feedback = apply(trainingBranch, feedbackFunc, TermList(term, desiredTaggedValue));
            // Resize the output of 'feedback' so that there is one output term per input
            resize_list(feedback_output(feedback), term->numInputs(), ANY_TYPE);

            // For each input which is trainable, send this feedback to it
            for (int i=0; i < term->numInputs(); i++) {
                Term* input = term->input(i);

                Term* outgoingFeedback = feedback_output(feedback)[i];

                // Initialize this field
                specialize_type(outgoingFeedback,
                        function_t::get_input_type(term->function, i));

                if (!is_trainable(input))
                    continue;

                // Set the weight on this term so that the sum weight for all outputs is 1
                set_feedback_weight(outgoingFeedback, 1.0f / numTrainableInputs);

                operation.sendFeedback(input, outgoingFeedback, DESIRED_VALUE_FEEDBACK);
            }
        }
    }
    //#endif
}

void refresh_training_branch(Branch& branch)
{
    refresh_training_branch(branch, default_training_branch(branch));
}

Branch& default_training_branch(Branch& branch)
{
    // Check if '#training' branch exists. Create if it doesn't exist
    if (!branch.contains(TRAINING_BRANCH_NAME))
        create_branch(branch, TRAINING_BRANCH_NAME);

    return nested_contents(branch[TRAINING_BRANCH_NAME]);
}

float get_feedback_weight(Term* term)
{
    return term->floatPropOptional("feedback-weight", 0);
}

void set_feedback_weight(Term* term, float weight)
{
    term->setFloatProp("feedback-weight", weight);
}

void feedback_register_constants(Branch& kernel)
{
#if 0
    FEEDBACK_TYPE = create_empty_type(kernel, "FeedbackType");
    DESIRED_VALUE_FEEDBACK = create_value(kernel, FEEDBACK_TYPE, "desired_value");
#endif
}

Branch& feedback_output(Term* term)
{
    // might refactor this:
    return nested_contents(term);
}
#endif

} // namespace circa
