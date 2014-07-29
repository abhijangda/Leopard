package registers;

public class Int64Register extends Register
{
	private long value;

	public Int64Register (byte _code)
	{
		type = RegisterType.Int64Register;
		code = _code;
	}
	
	public long getValue ()
	{
		return value;
	}
	public void setValue (long value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 8;
	}
}
