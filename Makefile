default : all;

run :
	(make -C kernel kernel.img)
	expect e0.tcl

test : run;

% :
	(make -C kernel $@)

clean:
	-(make -C kernel clean)
