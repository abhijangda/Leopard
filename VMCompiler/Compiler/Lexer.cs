using System;
using System.Collections.Generic;
using System.IO;

namespace Compiler
{
	public enum Tag
	{
		//Syntax Symbols
		DoubleQuotes, 
		SingleQuotes,
		Comma,
		StartBrace,
		EndBrace,
		StartParenthesis,
		EndParenthesis,
		StartBigBracket,
		EndBigBracket,
		EndOfLine,
		Dot, 
		Colon,

		//Unary Operators
		UnaryBitwiseNOTOperator, 
		UnaryNOTOperator,
	
		//Arithmetic Operator
		MultiplyOperator,
		AdditionOperator, 
		SubtractionOperator,
		DivisionOperator,
		ModulusOperator,
	
		//BitWise Operator
		RightShiftOperator, 
		LeftShiftOperator,
		AndOperator,
		XOROperator,
		OROperator,

		//Logical Operator
		LogicalAndOperator, 
		LogicalOROperator, 

		//Conditional Operator
		EqualityOperator,
		NotEqualToOperator,
		LessThanEqualToOperator,
		GreaterThanEqualToOperator,
		LessThan,
		GreaterThan,

		//Equal To Operator
		EqualOperator,

		//Built-in Types
		Void,
		Char,
		Short,
		Int,
		Float,
		Double,
		Long,

		//User Defined Types
		Class, 
		ClassType, 

		//Conditional Statements
		If,
		Else,

		//Loops
		While,
		Do,
		For,

		//Constants
		Number,
		Real,
		String,
		Character,

		ID,
		DynamicType,

		//Other Statements
		Return, 
		Break,
		Continue,

		New,

		//Access Modifiers
		Static, 
		Public,
		Private,
		Protected, 
	}

	public class Token
	{
		public Tag tag;
		protected char c;
		public static readonly Token StartBrace;
		public static readonly Token EndBrace;
		public static readonly Token StartParenthesis;
		public static readonly Token EndParenthesis;
		public static readonly Token StartBigBracket;
		public static readonly Token EndBigBracket;
		public static readonly Token EndOfLine;
		public static readonly Token Comma;
		public static readonly Token Colon;

		static Token ()
		{
			StartBrace = new Token ('{', Tag.StartBrace);
			EndBrace = new Token ('}', Tag.EndBrace);
			StartParenthesis = new Token ('(', Tag.StartParenthesis);
			EndParenthesis = new Token (')', Tag.EndParenthesis);
			StartBigBracket = new Token ('[', Tag.StartBigBracket);
			EndBigBracket = new Token (']', Tag.EndBigBracket);
			EndOfLine = new Token (';', Tag.EndOfLine);
			Comma = new Token (',', Tag.Comma);
			Colon = new Token (':', Tag.Colon);
		}

		public Token (char _c, Tag _tag)
		{
			c = _c;
			tag = _tag;
		}

		public override string ToString ()
		{
			return c.ToString ();
		}
	}

	public class Num : Token
	{
		public long value {get; protected set;}
		public Num (long _value) : base ('0', Tag.Number)
		{
			value = _value;
			tag = Tag.Number;
		}

		public override string ToString ()
		{
			return value.ToString ();
		}
	}

	public class CString : Token
	{
		public string value {get; protected set;}
		public CString (string value) : base ('s', Tag.String)
		{
			this.value = value;
			tag = Tag.String;
		}

		public override string ToString ()
		{
			return value;
		}
	}

	public class Character : Token
	{
		public char value {get; protected set;}

		public Character (char value) : base (value, Tag.Character)
		{
			this.value = value;
		}

		public override string ToString ()
		{
			return value.ToString ();
		}
	}

	public class Real : Token
	{
		public double value {get; protected set;}
		public Real (double _value) : base ('0', Tag.Real)
		{
			value = _value;
		}

		public override string ToString ()
		{
			return value.ToString ();
		}
	}

	public class Word : Token
	{
		public static readonly Word LogicalAnd;
		public static readonly Word LogicalOR;
		public static readonly Word UnaryBitwiseNOT;
		public static readonly Word UnaryNOT;
		public static readonly Word Multiply;
		public static readonly Word Addition;
		public static readonly Word Shift;
		public static readonly Word Relative;
		public static readonly Word And;
		public static readonly Word XOR;
		public static readonly Word OR;
		public static readonly Word Equality;
		public static readonly Word Equal;
		public static readonly Word LessThanEqualTo;
		public static readonly Word GreaterThanEqualTo;
		public static readonly Word NotEqualTo;
		public static readonly Word RightShift;
		public static readonly Word LeftShift;
		public static readonly Word Subtraction;
		public static readonly Word Modulus;
		public static readonly Word Division;
		public static readonly Word LessThan;
		public static readonly Word GreaterThan;
		public static readonly Word Return;
		public static readonly Word Break;
		public static readonly Word Continue;
		public static readonly Word Class;
		public static readonly Word Dot;
		public static readonly Word New;
		public static readonly Word Static;
		public static readonly Word Public;
		public static readonly Word Private;
		public static readonly Word Protected;

		public string value {get; protected set;}

		static Word ()
		{
			Class = new Word ("class", Tag.Class);
			Return = new Word ("return", Tag.Return);
			Break = new Word ("break", Tag.Break);
			Continue = new Word ("continue", Tag.Continue);

			LogicalAnd = new Word ("&&", Tag.LogicalAndOperator);
			LogicalOR = new Word ("||", Tag.LogicalOROperator);

			UnaryBitwiseNOT = new Word ("~", Tag.UnaryBitwiseNOTOperator);
			UnaryNOT = new Word ("!", Tag.UnaryNOTOperator);
			Multiply = new Word ("*", Tag.MultiplyOperator);
			Addition = new Word ("+", Tag.AdditionOperator);
			Subtraction = new Word ("-", Tag.SubtractionOperator);
			Modulus = new Word ("%", Tag.ModulusOperator);
			Division = new Word ("/", Tag.DivisionOperator);
			RightShift = new Word (">>", Tag.RightShiftOperator);
			LeftShift = new Word ("<<", Tag.LeftShiftOperator);
			And = new Word ("&", Tag.AndOperator);
			XOR = new Word ("^", Tag.XOROperator);
			OR = new Word ("|", Tag.OROperator);
			Equality = new Word ("==", Tag.EqualityOperator);
			NotEqualTo = new Word ("!=", Tag.NotEqualToOperator);
			LessThan = new Word ("<", Tag.LessThan);
			GreaterThan = new Word (">", Tag.GreaterThan);
			LessThanEqualTo = new Word ("<=", Tag.LessThanEqualToOperator);
			GreaterThanEqualTo = new Word (">=", Tag.GreaterThanEqualToOperator);

			Equal = new Word ("=", Tag.EqualOperator);
			Dot = new Word (".", Tag.Dot);
			New = new Word ("new", Tag.New);
			Static = new Word ("static", Tag.Static);
			Public = new Word ("public", Tag.Public);
			Private = new Word ("private", Tag.Private);
			Protected = new Word ("protected", Tag.Protected);			
		}

		public Word (string s, Tag _tag) : base ('0', _tag)
		{
			value = s;
		}

		public override string ToString ()
		{
			return value;
		}
	}

	public class Type : Word
	{
		public static readonly Type Integer;
		public static readonly Type Character;
		public static readonly Type Short;
		public static readonly Type Long;
		public static readonly Type Double;
		public static readonly Type Void;
		public static readonly Type Float;
		public virtual int width {get; set;}

		static Type ()
		{
			Integer = new Type ("int", Tag.Int, 4);
			Character = new Type ("char", Tag.Char, 1);
			Short = new Type ("short", Tag.Short, 2);
			Long = new Type ("long", Tag.Long, 8);
			Double = new Type ("double", Tag.Double, 8);
			Void = new Type ("void", Tag.Void, 1);
			Float = new Type ("float", Tag.Float, 4);
		}

		public Type (string s, Tag _tag, int w) : base (s, _tag)
		{
			width = w;
		}

		protected Type (int w) : base ("runtime_type", Compiler.Tag.DynamicType)
		{
			width = w;
		}

		public static bool numeric (Type t)
		{
			if (t == Type.Character || t == Type.Integer || t == Type.Float || t == Type.Double)
				return true;
			return false;
		}

		public static Type max (Type t1, Type t2)
		{
			if (! numeric (t1) || ! numeric (t2))
				return null;

			if (t1.width > t2.width)
				return t1;

			else 
				return t2;
		}

		public static Type min (Type t1, Type t2)
		{
			if (! numeric (t1) || ! numeric (t2))
				return null;

			if (t1.width < t2.width)
				return t1;

			else 
				return t2;
		}
	}

	public class Lexer
	{
		public int line {get; private set;}
		char peek = ' ';
		Dictionary <string, Word> words;
		int pos;
		String fileString;

		public Lexer ()
		{
			words = new Dictionary<string, Word> ();

			reserve (new Word ("if", Tag.If));
			reserve (new Word ("else", Tag.Else));
			reserve (new Word ("while", Tag.While));
			reserve (new Word ("do", Tag.Do));
			reserve (new Word ("for", Tag.For));
			reserve (Word.Return);
			reserve (Word.Continue);
			reserve (Word.Break);
			reserve (Word.New);
			reserve (Word.Static);
			reserve (Word.Public);
			reserve (Word.Private);
			reserve (Word.Protected);
			reserve (Type.Integer);
			reserve (Type.Character);
			reserve (Type.Short);
			reserve (Type.Long);
			reserve (Type.Double);
			reserve (Type.Void);
			reserve (Type.Float);
			reserve (Type.Class);

			//strLine = Console.ReadLine ();
			//pos = 0;
			fileString = File.ReadAllText ("./../../test");
			pos = 0;
			line = 1;
		}

		private void reserve (Word w)
		{
			words.Add (w.value, w);
		}

		void readCharacter ()
		{
			if (pos < fileString.Length)
			{	
				peek = fileString [pos];
				pos ++;
			}

//			peek = (char)Console.Read ();
		}

		void pushBackPeek ()
		{
			pos--;
		}

		bool readNext (char c)
		{
			readCharacter ();
			if (peek != c)
				return false;

			peek = ' ';
			return true;
		}

		public Token scan ()
		{
			peek = ' ';
			for (; ; readCharacter ())
			{
				if (peek == ' ' || peek == '\t')
					continue;

				else if (peek == '\n')
					line = line + 1;

				else
					break;
			}

			char oldpeek = peek;

			switch (peek)
			{
			case ':':
				return Token.Colon;
			case '\"':
				readCharacter ();
				string s = "";
				while (true)
				{
					if ((oldpeek == '\\' && peek != '\"') || (oldpeek != '\\' && peek == '\"'))
						return new CString (s);

					if (peek == '\\')
					{
						oldpeek = peek;
						readCharacter ();
						continue;
					}

					s += peek.ToString ();
					oldpeek = peek;
					readCharacter ();
				}

			case '\'':
				readCharacter ();
				oldpeek = peek;
				Console.WriteLine ("PEEK IS " + peek);
				if (peek == '\\')
				{
					readCharacter ();
					if (peek != '\'' || peek != '\"' || peek != '\\')
					{
						Console.WriteLine ("Error: unexpected character \"" + peek);
						return null;
					}
					oldpeek = peek;
				}
				readCharacter ();
				Console.WriteLine ("PEEK IS " + peek);
				if (peek != '\'')
				{
					Console.WriteLine ("Error expected '");
					return null;
				}
				Console.WriteLine ("PEEK IS " + peek);
				return new Character (oldpeek);
			case '&':
				readCharacter ();
				if (peek == '&')
					return Word.LogicalAnd;

				return Word.And;

			case '|':
				readCharacter ();
				if (peek == '|')
					return Word.LogicalOR;

				return Word.OR;

			case '~':
				return Word.UnaryNOT;

			case '^':
				return Word.XOR;

			case '+':
				return Word.Addition;

			case '-':
				return Word.Subtraction;

			case '*':
				return Word.Multiply;

			case '/':
				return Word.Division;
			
			case '%':
				return Word.Modulus;

			case '<':
				readCharacter ();
				if (peek == '=')
					return Word.LessThanEqualTo;

				if (peek == '<')
					return Word.LeftShift;

				return Word.LessThan;

			case '>':
				readCharacter ();
				if (peek == '=')
					return Word.GreaterThanEqualTo;

				if (peek == '>')
					return Word.RightShift;

				return Word.GreaterThan;
			
			case '=':
				readCharacter ();
				if (peek == '=')
					return Word.Equality;

				return Word.Equal;

			case '{':
				return Token.StartBrace;
			
			case '}':
				return Token.EndBrace;

			case '(':
				return Token.StartParenthesis;

			case ')':
				return Token.EndParenthesis;

			case '[':
				return Token.StartBigBracket;

			case ']':
				return Token.EndBigBracket;

			case ';':
				return Token.EndOfLine;

			case ',':
				return Token.Comma;
			}

			if (char.IsDigit (peek))
			{
				long v = 0;
				do
				{
					v = 10*v + int.Parse (peek.ToString ());
					readCharacter ();
				}
				while (char.IsDigit (peek));

				if (peek != '.') 
				{
					pushBackPeek ();
					return new Num (v);
				}

				double x = v;
				double d = 10;
				for (;;)
				{
					readCharacter ();
					if (!char.IsDigit (peek))
						break;

					x = x + int.Parse (peek.ToString ()) / d;
					d = d * 10;
				}
				pushBackPeek ();
				return new Real (x);
			}

			if (char.IsLetter (peek))
			{
				String s = "";
				do
				{
					s += peek.ToString ();
					readCharacter ();
				}
				while (char.IsLetterOrDigit (peek) || peek == '_');

				if (words.ContainsKey (s))
				{
					pushBackPeek ();
					return words [s];
				}
				Word w = new Word (s, Tag.ID);
				words.Add (s, w);
				pushBackPeek ();
				return w;
			}

			Token t;
			if (peek == '$')
				t = new Token (peek, Tag.EndOfLine);
			else if (peek == '.')
				t = Word.Dot;
			else
			    t = new Token (peek, Tag.ID);
			peek = ' ';
			return t;
		}
	}
}