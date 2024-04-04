#include "metaffi_primitives.h"

metaffi_char8::operator metaffi_char16() const
{
	metaffi_char16 c16{};
	int8_t required_bytes = num_of_bytes(this->c);
	
	if(required_bytes == -1)
	{
		throw std::invalid_argument("Invalid UTF-8 character");
	}
	
	uint32_t codepoint = 0;
	if(required_bytes == 1)
	{
		codepoint = this->c[0];
	}
	else if(required_bytes == 2)
	{
		codepoint = ((this->c[0] & 0x1F) << 6) | (this->c[1] & 0x3F);
	}
	else if(required_bytes == 3)
	{
		codepoint = ((this->c[0] & 0x0F) << 12) | ((this->c[1] & 0x3F) << 6) | (this->c[2] & 0x3F);
	}
	else if(required_bytes == 4)
	{
		codepoint = ((this->c[0] & 0x07) << 18) | ((this->c[1] & 0x3F) << 12) | ((this->c[2] & 0x3F) << 6) |
		            (this->c[3] & 0x3F);
	}
	else
	{
		throw std::invalid_argument("Invalid UTF-8 character");
	}
	
	if(codepoint <= 0xFFFF)
	{
		c16.c[0] = static_cast<char16_t>(codepoint);
		c16.c[1] = u'\0';
	}
	else
	{
		codepoint -= 0x10000;
		c16.c[0] = static_cast<char16_t>((codepoint >> 10) + 0xD800);
		c16.c[1] = static_cast<char16_t>((codepoint % 0x400) + 0xDC00);
	}
	
	// Return the metaffi_char16 object
	return c16;
}

metaffi_char8::operator metaffi_char32() const
{
	char32_t c32 = 0;
	int8_t required_bytes = num_of_bytes(this->c);
	
	if(required_bytes == -1)
	{
		throw std::invalid_argument("Invalid UTF-8 character");
	}
	
	if(required_bytes == 1)
	{
		c32 = this->c[0];
	}
	else if(required_bytes == 2)
	{
		c32 = ((this->c[0] & 0x1F) << 6) | (this->c[1] & 0x3F);
	}
	else if(required_bytes == 3)
	{
		c32 = ((this->c[0] & 0x0F) << 12) | ((this->c[1] & 0x3F) << 6) | (this->c[2] & 0x3F);
	}
	else if(required_bytes == 4)
	{
		c32 = ((this->c[0] & 0x07) << 18) | ((this->c[1] & 0x3F) << 12) | ((this->c[2] & 0x3F) << 6) | (this->c[3] & 0x3F);
	}
	
	return metaffi_char32(c32);
}

metaffi_char16::operator metaffi_char8() const
{
    metaffi_char8 c8{};
    int8_t required_bytes = num_of_bytes(this->c);

    if(required_bytes == -1)
    {
        throw std::invalid_argument("Invalid UTF-16 character");
    }

    uint32_t codepoint = 0;
    if(required_bytes == 2)
    {
        codepoint = this->c[0];
    }
    else if(required_bytes == 4)
    {
        codepoint = ((this->c[0] - 0xD800) << 10) + (this->c[1] - 0xDC00) + 0x10000;
    }

    if(codepoint <= 0x7F)
    {
        c8.c[0] = static_cast<char8_t>(codepoint);
    }
    else if(codepoint <= 0x7FF)
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 6) | 0xC0);
        c8.c[1] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }
    else if(codepoint <= 0xFFFF)
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 12) | 0xE0);
        c8.c[1] = static_cast<char8_t>(((codepoint >> 6) & 0x3F) | 0x80);
        c8.c[2] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }
    else
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 18) | 0xF0);
        c8.c[1] = static_cast<char8_t>(((codepoint >> 12) & 0x3F) | 0x80);
        c8.c[2] = static_cast<char8_t>(((codepoint >> 6) & 0x3F) | 0x80);
        c8.c[3] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }

    return c8;
}

metaffi_char16::operator metaffi_char32() const
{
    char32_t c32 = {0};
    int8_t required_bytes = num_of_bytes(this->c);

    if(required_bytes == -1)
    {
        throw std::invalid_argument("Invalid UTF-16 character");
    }

    if(required_bytes == 2)
    {
        c32 = this->c[0];
    }
    else if(required_bytes == 4)
    {
        c32 = ((this->c[0] - 0xD800) << 10) + (this->c[1] - 0xDC00) + 0x10000;
    }

    return metaffi_char32(c32);
}

metaffi_char32::operator metaffi_char8() const
{
    metaffi_char8 c8{};
    uint32_t codepoint = this->c;

    if(codepoint <= 0x7F)
    {
        c8.c[0] = static_cast<char8_t>(codepoint);
    }
    else if(codepoint <= 0x7FF)
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 6) | 0xC0);
        c8.c[1] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }
    else if(codepoint <= 0xFFFF)
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 12) | 0xE0);
        c8.c[1] = static_cast<char8_t>(((codepoint >> 6) & 0x3F) | 0x80);
        c8.c[2] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }
    else
    {
        c8.c[0] = static_cast<char8_t>((codepoint >> 18) | 0xF0);
        c8.c[1] = static_cast<char8_t>(((codepoint >> 12) & 0x3F) | 0x80);
        c8.c[2] = static_cast<char8_t>(((codepoint >> 6) & 0x3F) | 0x80);
        c8.c[3] = static_cast<char8_t>((codepoint & 0x3F) | 0x80);
    }

    return c8;
}

metaffi_char32::operator metaffi_char16() const
{
    metaffi_char16 c16{};
    uint32_t codepoint = this->c;

    if(codepoint <= 0xFFFF)
    {
        c16.c[0] = static_cast<char16_t>(codepoint);
        c16.c[1] = u'\0';
    }
    else
    {
        codepoint -= 0x10000;
        c16.c[0] = static_cast<char16_t>((codepoint >> 10) + 0xD800);
        c16.c[1] = static_cast<char16_t>((codepoint % 0x400) + 0xDC00);
    }

    return c16;
}