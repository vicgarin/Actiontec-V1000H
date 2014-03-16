/*****************************************************************************
Interface jnt.Bench.Target
 *****************************************************************************/
package jnt.Bench;

/** 
Interface for a Benchmark Target.
Code to be measured by the Bench framework should provide a class implementing
this interface.  Place the code to be measured in the execute method.

@author Bruce R. Miller (bruce.miller@nist.gov)
@author Contribution of the National Institute of Standards and Technology,
@author not subject to copyright.
*/
public interface Target {
  /** The code to be measured is placed in this method.
    * @return null lets jnt.Bench.Bench handle the timings.
    * Otherwise, return an array containing the one or more measured values.
    * @see jnt.Bench.Bench [start|stop|reset]Timer methods for measurement tools.
    */    
  public double[] execute(Bench bench) throws Exception;
}
