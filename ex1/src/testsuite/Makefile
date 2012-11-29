all:
	make -B -C ../generator
	make -B -C ../serial
	make -B -C ../mpi/ptp/continuous-single 
	make -B -C ../mpi/ptp/hybrid 
	make -B -C ../mpi/collective/hybrid
	make -B -C ../mpi/collective/continuous-single
	make -B -C ../openmp



debug:
	make -B DEBUG=y -C ../generator
	make -B DEBUG=y -C ../serial
	make -B DEBUG=y -C ../mpi/ptp/continuous-single 
	make -B DEBUG=y -C ../mpi/ptp/hybrid 
	make -B DEBUG=y -C ../mpi/collective/hybrid
	make -B DEBUG=y -C ../mpi/collective/continuous-single
	make -B DEBUG=y -C ../openmp
