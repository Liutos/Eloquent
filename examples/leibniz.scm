(set cc (lambda (test)
  (/ 1
     (i2d (pred (* 2 test))))))

(set l-rec (lambda (n test)
  (set c (cc test))
  (if (>= test n)
      c
    (- c (l-rec n (succ test))))))

(set l (lambda (n)
  (* 4 (l-rec n 1))))

(l 100)
