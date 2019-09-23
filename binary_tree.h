#pragma once

#include <charconv>
#include <ctime>
#include <functional>

namespace dbj {

	// dbj::system_call("@chcp 65001")
	bool system_call( const char * cmd_ ) {
		_ASSERTE(cmd_);
		if (0 != system(NULL)) {
			if (-1 == system( cmd_ ))// utf-8 codepage! 
			{
				switch (errno) {
				case E2BIG:		perror("The argument list(which is system - dependent) is too big"); break;
				case ENOENT:	perror("The command interpreter cannot be found."); break;
				case ENOEXEC:	perror("The command - interpreter file cannot be executed because the format is not valid."); break;
				case ENOMEM:	perror("Not enough memory is available to execute command; or available memory has been corrupted; or a non - valid block exists, which indicates that the process that's making the call was not allocated correctly."); break;
				}
				return false;
			}
			return true;
		}
		return false;
	}
}

namespace dbj_tree_research
{
	using namespace ::std;

	/* quick utils --------------------------------------------------------------------------- */
	inline auto random = [](size_t max_val, size_t min_val = 1)
		-> std::size_t {
				_ASSERTE(min_val < max_val);
		static auto initor = []() {
			std::srand((unsigned)std::time(nullptr));
			return 0;
		}();
		return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
	};

	struct num_to_str final 
	{
		std::array<char, 0xFF> str{ { char(0) } };
		static const std::errc not_posix_error = std::errc();

		template< typename N>
		char const* operator() (N const& number_) noexcept
		{
			static_assert(is_arithmetic_v<N>);
			if (auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), number_);
				ec == not_posix_error) {
				return str.data();
			}
			else {
				return nullptr;
			}
		}
	}; // num_to_str

	/* BTree node --------------------------------------------------------------------------- */

	template <
		typename T, 
		/* less then */
		typename Comparator_functor ,
		/* string reprezentation of the value */
		typename String_functor
	>
	class tree_node_t final
	{
		T val;
		tree_node_t* left{};
		tree_node_t* right{};

		Comparator_functor	comparator;
		String_functor		stringizer;

	public:
		using type = tree_node_t;
		using value_type = T;

		tree_node_t(value_type const& x) noexcept : val(x) {}

		// self erasing
		~tree_node_t() {
			if (left != nullptr) {
				delete left;
				left = nullptr;
			}
			if (right != nullptr) {
				delete right;
				right = nullptr;
			}
		}

		void append(const value_type& new_data)
		{
			value_type this_data = this->val;

			if (comparator( new_data , this_data ) ) // need to do left append
			{
				if (left != nullptr)
					left->append(new_data);
				else
					left = new type(new_data);
			} else
			{ 
				if (right != nullptr)
					right->append(new_data);
				else
					right = new type(new_data);
			}
		}

		void pretty_print
		( string prefix = u8"", bool isLeft = true )
		{
			if (right)
				 right->pretty_print( prefix + (isLeft ? u8"│   " : u8"    "), false);

			std::printf(u8"%s%s\n",
				(prefix + (isLeft ? u8"└── " : u8"┌── ")).c_str(),
				stringizer(this->val)
			);

			if (left)
				left->pretty_print( prefix + (isLeft ? u8"    " : u8"│   "), true);
		}


		/* if callback returns false, processing stops */
		template< typename callable_object >
		void pre_proc(callable_object cb_) {
			if ( ! cb_( this->val ) ) return ;
			if (right) right->pre_proc( cb_);
			if (left) left->pre_proc(cb_);
		}

		template< typename callable_object >
		void in_proc(callable_object cb_) {
			if (right) right->in_proc(cb_);
			if (!cb_(this->val)) return;
			if (left) left->in_proc(cb_);
		}

		template< typename callable_object >
		void post_proc(callable_object cb_) {
			if (right) right->post_proc(cb_);
			if (!cb_(this->val)) return;
			if (left) left->post_proc(cb_);
		}

	}; // tree node

	// legal_call_back
	bool by_ten(int& v) { v *= 10; return true;  };
	bool print(int& v) { printf(" %d ", v); return true;  };

	void simple_tree_test ()
	{
		dbj::system_call("@chcp 65001");

		auto intless = [](auto a, auto b) -> bool { return a < b;  };

			using binary_sorted_tree = tree_node_t< int, std::less<int> , num_to_str >;
			binary_sorted_tree root = binary_sorted_tree(6);

			//for (int k = 1; k < 0xF; k++)
			//{
			//	root.append( binary_sorted_tree::value_type( random(size_t(0xFF))) );
			//}

			root.append(1);
			root.append(9);
			root.append(2);
			root.append(8);
			root.append(3);
			root.append(7);
			root.append(4);
			root.append(5);

			std::printf("\n");
			root.pretty_print();
			std::printf("\nPRE PROC\t");
			root.pre_proc(print);
			std::printf("\nIN PROC\t\t");
			root.in_proc(print);
			std::printf("\nPOST PROC\t");
			root.post_proc(print);
			std::printf("\n");
	}

} // namespace dbj_tree_research