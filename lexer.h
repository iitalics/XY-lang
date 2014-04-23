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
	
	
	struct token
	{
		token (int tok_ = '\0');
		
		int tok;
		std::string str;
		number num;
		
		std::string to_str () const;
		inline bool eof () const { return tok == '\0'; }
		
		enum
		{
			number_token = 1000,
			symbol_token,
			
			_two_chars,
			seq_token,
			eql_token,
			neq_token,
			gre_token,
			lse_token,
			
			keyword__start = 2000,
			keyword_let = keyword__start,
			keyword_struct,
			keyword_true, keyword_false,
			keyword_or, keyword_and
		};
		
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
	};
	
	
	const token& current () const;
	bool advance ();
	void trim_left ();
	
private:
	typedef std::unique_ptr<std::istream> inp_stream;
	
	
	state& parent;
	
	inp_stream input;
	void input_opened ();
	
	char peek_char;
	int line;
	bool is_eof;
	
	token current_token;
	
	static bool is_sym_char (char c);
	bool parse_digit (char c, int base, int& out);
	bool parse_num ();
	bool parse_sym ();
};


};
