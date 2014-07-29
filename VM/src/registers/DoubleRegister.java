package registers;

public abstract class DoubleRegister extends Register
{
	private double value;

	public DoubleRegister (byte _code)
	{
		type = RegisterType.DoubleRegister;
		code = _code;
	}
	
	public double getValue ()
	{
		return value;
	}
	public void setValue (double value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 8;
	}
}
