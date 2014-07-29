package registers;

public class Int32Register extends Register
{
	private int value;

	public Int32Register (byte _code)
	{
		type = RegisterType.Int32Register;
		code = _code;
	}
	
	public int getValue ()
	{
		return value;
	}
	public void setValue (int value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 4;
	}
}
