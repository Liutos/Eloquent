(set f (lambda () (dget special-variable)))

(set g1 (lambda () (dset special-variable 233) (f)))

(g1) ; 233

(set g2 (lambda () (dset special-variable 666) (f)))

(g2) ; 666
