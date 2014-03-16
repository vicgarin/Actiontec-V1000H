package jnt.scimark2;

/*****************************************************************************
 Main benchmark driver
 *****************************************************************************/


public class applet implements jnt.Bench.Target
{

  public double[] execute(jnt.Bench.Bench b)
  {
		double res[] = new double[6];
		Random R = new Random(Constants.RANDOM_SEED);
		double resolution_time =  Constants.RESOLUTION_DEFAULT;


		// preload benchmark components to minimize network download
		// time.  (These sizes are not use for benchmarking,)

		double TINY_RES_TIME = 0.00001;	
		b.noteStatus("Downloading FFT");
		res[1] = kernel.measureFFT( Constants.TINY_FFT_SIZE, TINY_RES_TIME, R);
		Thread.yield();

		b.noteStatus("Downloading SOR");
		res[2] = kernel.measureSOR( Constants.TINY_SOR_SIZE, TINY_RES_TIME, R);
		Thread.yield();

		b.noteStatus("Downloading Monte Carlo");
		res[3] = kernel.measureMonteCarlo(TINY_RES_TIME, R);
		Thread.yield();


		b.noteStatus("Downloading Sparse Matrix Multplication");
		res[4] = kernel.measureSparseMatmult( Constants.TINY_SPARSE_SIZE_M, 
						Constants.TINY_SPARSE_SIZE_nz, 
						TINY_RES_TIME, R);
		Thread.yield();

		b.noteStatus("Downloading LU factorization");
		res[5] = kernel.measureLU( Constants.TINY_LU_SIZE, TINY_RES_TIME, R);
		Thread.yield();


		// now begin benchmarking

		b.noteStatus("benchmarking FFT");
		res[1] = kernel.measureFFT( Constants.FFT_SIZE, resolution_time, R);

		b.noteStatus("benchmarking SOR");
		res[2] = kernel.measureSOR( Constants.SOR_SIZE, resolution_time, R);

		b.noteStatus("benchmarking Monte Carlo");
		res[3] = kernel.measureMonteCarlo(resolution_time, R);


		b.noteStatus("benchmarking Sparse Matrix Multplication");
		res[4] = kernel.measureSparseMatmult( Constants.SPARSE_SIZE_M, 
						Constants.SPARSE_SIZE_nz, 
						resolution_time, R);

		b.noteStatus("benchmarking LU factorization");
		res[5] = kernel.measureLU( Constants.LU_SIZE, resolution_time, R);

		res[0] = (res[1] + res[2] + res[3] + res[4] + res[5]) / 5;


		
		// if either the FFT or LU did not validate, then we have
		// an invalid JVM (i.e. one with an incorrect floating point
		// model).  By returning 0.0, this avoids posting artificically
		// high SciMark scores for systems that may be cheating.
		//
		if (res[1] == 0.0 || res[5] == 0.0)
		{
			res[0] = res[1] = res[2] = res[3] = res[4] = res[5] = 0.0;
		}

		return res;
	}
}

