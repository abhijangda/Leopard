package registers;

public class ASCIICharacterRegister extends Register
{
	private char value;

	public ASCIICharacterRegister (byte _code)
	{
		type = RegisterType.Int8Register;
		code = _code;
	}
	
	public char getValue ()
	{
		return value;
	}
	public void setValue (char value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 1;
	}
}
