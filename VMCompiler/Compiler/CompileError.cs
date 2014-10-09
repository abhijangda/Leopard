using System;

namespace Compiler
{
	public class CompileError
	{
		string message;
		int line;

		public CompileError (string message, int line)
		{
			this.line = line;
			this.message = message;
		}

		public override string ToString ()
		{
			return "Error at " + line.ToString () + ": " + this.message;
		}
	}
}

