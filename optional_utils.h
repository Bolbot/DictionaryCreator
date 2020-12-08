#ifndef __OPTIONAL_UTILS_H__
#define __OPTIONAL_UTILS_H__

// Theese things are not yet needed so they will stay in this file that isn't necessary neither to compile not to link
// Features aren't in fact deprecated, they're just unnecessary

namespace [[ deprecated ]] lowercase_traits
{
	template <typename Char>
	struct lower_case_char_traits : public std::char_traits<Char>
	{
		static void assign(Char &dest, const Char &source)
		{
			if constexpr (std::is_same_v<CharType, char>)
			{
				dest = std::tolower(source);
			}
			else
			{
				dest = std::towlower(source);
			}
		}

		static Char *move(Char *dest, const Char *source, size_t size)
		{
			for (size_t i = 0; i != size; ++i)
			{
				assign(dest[i], source[i]);
			}
			return dest;
		}

		static Char *copy(Char *dest, const Char *source, size_t size)
		{
			return move(dest, source, size);
		}

		static bool eq(Char a, Char b)
		{
			if constexpr (std::is_same_v<Char, char>)
			{
				return tolower(a) == tolower(b);
			}
			else
			{
				return towlower(a) == towlower(b);
			}
		}

		static bool lt(Char a, Char b)
		{
			if constexpr (std::is_same_v<Char, char>)
			{
				return tolower(a) < tolower(b);
			}
			else
			{
				return towlower(a) < towlower(b);
			}
		}

		static int compare(const Char *a, const Char *b, size_t n)
		{
			for (; n != 0; ++a, ++b, --n)
			{
				if (lt(*a, *b))
				{
					return -1;
				}
				else if (lt(*b, *a))
				{
					return 1;
				}
			}
			return 0;
		}

		static const Char *find(const Char *heystack, size_t size, Char needle)
		{
			for (size_t i = 0; i != size; ++i)
			{
				if (eq(heystack[i], needle))
				{
					return &heystack[i];
				}
			}
			return nullptr;
		}
	};

	template <typename Char, typename Traits, typename AnotherChar>
	std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &out, const std::basic_string<AnotherChar> &str)
	{
		return out.write(str.data(), str.size());
	}
}

#endif
