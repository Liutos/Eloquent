(define fact (n)
  (if (>= n 2)
      (* n (fact (pred n)))
    1))

(trace fact)

(fact 5)
