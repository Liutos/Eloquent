(define fact (n)
  (if (>= n 2)
      (* n (fact (pred n)))
    1))

(fact 5)
