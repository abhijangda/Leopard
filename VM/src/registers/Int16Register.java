package registers;

public class Int16Register extends Register
{
	private short value;

	public Int16Register (byte _code)
	{
		type = RegisterType.Int32Register;
		code = _code;
	}
	
	public short getValue ()
	{
		return value;
	}
	public void setValue (short value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 2;
	}
}
