(set mod (lambda (n d)
  (if (= n d)
      0
    (if (>= n d)
	(mod (- n d) d)
      n))))

(set is_prime-rec (lambda (n test)
  (if (= (* test test) n)
      0
    (if (>= (* test test) n)
        1
      (if (= (mod n test) 0)
	  0
	(is_prime-rec n (succ test)))))))

(set is_prime (lambda (n)
  (is_prime-rec n 2)))

(is_prime 997)
