package registers;

public class FloatRegister extends Register 
{
	private float value;

	public FloatRegister (byte _code)
	{
		type = RegisterType.FloatRegister;
		code = _code;
	}
	
	public float getValue ()
	{
		return value;
	}
	public void setValue (float value)
	{
		this.value = value;
	}
	
	public int getSize ()
	{
		return 4;
	}
}
