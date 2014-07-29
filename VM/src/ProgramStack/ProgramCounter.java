package ProgramStack;

public class ProgramCounter 
{
	long value;
	
	public ProgramCounter (long _value)
	{
		this.value = _value;
	}
	
	public ProgramCounter ()
	{
		value = 0;
	}
	
	public void increment ()
	{
		value++;
	}
	
	public void decrement ()
	{
		value--;
	}
	
	public long getValue ()
	{
		return value;
	}
}