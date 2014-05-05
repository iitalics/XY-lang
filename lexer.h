#pragma once

namespace xy {

class state;
class lexer
{
public:
	lexer (state& parent_);
	~lexer ();
	
	bool open (const std::string& filename);
	bool open_string (const std::string& data);
	
	char peek ();
	char read ();
	bool eof () const;
	
	int line_num () const;
	int col_num () const; // this function currently doesn't work correctly
	std::string file () const;
	
	struct token
	{
		enum
		{
			eof_token = 0,
			number_token = 1000,
			symbol_token,
			string_token,
			
			_two_chars,
			seq_token,
			eql_token,
			neq_token,
			gre_token,
			lse_token,
			rarr_token,
			
			keyword__start = 2000,
			keyword_let = keyword__start,
			keyword_with, keyword_use, 
			keyword_true, keyword_false,
			keyword_or, keyword_and
		};
		
		token (int tok_ = eof_token);
		
		int tok;
		std::string str;
		number num;
		
		std::string to_str () const;
		bool is_unary_op () const;
		bool is_binary_op () const;
		bool is_expression () const;
		
		inline bool eof () const { return tok == eof_token; }
		
		struct two_char
		{
			char a, b;
			int tok;
			
			inline two_char (const std::string& str, int t)
				: a(str[0]), b(str[1]), tok(t) {}
			inline constexpr two_char ()
				: a('\0'), b('\0'), tok('\0') {}
			
			std::string str () const;
		};
		
		static std::vector<std::string> keywords;
		static std::vector<two_char> two_chars;
		
		static inline std::string eof_string () { return "<eof>"; }
		static inline std::string number_string () { return "<number>"; }
		static inline std::string symbol_string () { return "<symbol>"; }
		static inline std::string string_string () { return "<string>"; }
	};
	
	
	const token& current () const;
	bool advance ();
	void trim_left ();
	
	bool expect (int tok, bool adv = false);
	bool unexpect ();
	
private:
	typedef std::unique_ptr<std::istream> inp_stream;
	
	
	state& parent;
	
	inp_stream input;
	bool input_opened ();
	
	char peek_char;
	bool is_eof;
	
	int line;
	int col, col_display;
	std::string file_name;
	
	token current_token;
	
	static bool is_sym_char (char c);
	bool parse_digit (char c, int base, int& out);
	bool parse_num ();
	bool parse_sym ();
	bool parse_str ();
};


};
