all:
	make -B -C ../generator
	make -B -C ../serial
	make -B -C ../mpi/continuous
	make -B -C ../mpi/cyclic
	make -B -C ../openmp



debug:
	make -B DEBUG=y -C ../generator
	make -B DEBUG=y -C ../serial
	make -B DEBUG=y -C ../mpi/continuous
	make -B DEBUG=y -C ../mpi/cyclic
	make -B DEBUG=y -C ../openmp
