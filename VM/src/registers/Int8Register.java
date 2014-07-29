package registers;

public class Int8Register extends Register
{
	private byte value;

	public Int8Register (byte _code)
	{
		type = RegisterType.Int8Register;
		code = _code;
	}
	
	public byte getValue ()
	{
		return value;
	}
	public void setValue (byte value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 1;
	}
}