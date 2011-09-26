// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "debug.h"
#include "token_stream.h"

namespace circa {

token::Token const&
TokenStream::next(int lookahead) const
{
    int i = this->_position + lookahead;

    if (i >= (int) tokens.size())
        internal_error("index out of bounds");

    if (i < 0)
        internal_error("index < 0");

    return tokens[i];
}

int
TokenStream::findNextNonWhitespace(int lookahead) const
{
    int index = this->_position;

    while (true) {

        if (index >= (int) tokens.size())
            return -1;

        if (tokens[index].match == token::WHITESPACE) {
            index++;
            continue;
        }

        if (lookahead == 0)
            return index;

        lookahead--;
        index++;
    }
}

int
TokenStream::nextNonWhitespace(int lookahead) const
{
    int index = findNextNonWhitespace(lookahead);

    if (index == -1)
        return token::EOF_TOKEN;

    return tokens[index].match;
}

bool TokenStream::nextIs(int match, int lookahead) const
{
    if ((this->_position + lookahead) >= tokens.size())
        return false;
        
    return next(lookahead).match == match;
}

std::string
TokenStream::consume(int match)
{
    if (finished())
        internal_error(std::string("Unexpected EOF while looking for ")
                + token::get_token_text(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << token::get_token_text(match)
            << ", found " << token::get_token_text(next().match)
            << " '" << next().text << "')";
        internal_error(msg.str());
    }

    return tokens[_position++].text;
}

bool
TokenStream::nextNonWhitespaceIs(int match, int lookahead) const
{
    return nextNonWhitespace(lookahead) == match;
}

int
TokenStream::getPosition() const
{
    return _position;
}

void
TokenStream::resetPosition(int loc)
{
    ca_assert(loc >= 0);
    _position = loc;
}

std::string
TokenStream::toString() const
{
    std::stringstream out;

    out << "{index: " << _position << ", ";
    out << "tokens: [";

    bool first = true;

    for (unsigned int i=0; i < tokens.size(); i++) {
        if (!first) out << ", ";
        out << tokens[i].toString();
        first = false;
    }
    out << "]}";
    return out.str();
}

void print_remaining_tokens(std::ostream& out, TokenStream& tokens)
{
    for (int i=0; i < tokens.remaining(); i++) {
        if (i != 0) out << " ";
        out << token::get_token_text(tokens.next(i).match);
        out << "(" << tokens.next(i).text << ")";
    }
}

} // namespace circa

