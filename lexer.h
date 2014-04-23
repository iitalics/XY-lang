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
		
		enum
		{
			number_token = 256,
			symbol_token
		};
		
		std::string to_str () const;
		inline bool eof () const { return tok == '\0'; }
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
	
	bool parse_digit (char c, int base, int& out) const;
	bool is_sym_char (char c) const;
	bool parse_num ();
	bool parse_sym ();
};


};
