#lang racket/base

(define (read-all)
  (let ([s (read)])
    '(displayln s)
    (cond [(not (eof-object? s)) 
           (begin (displayln s) (displayln (string-length s)) (read-all))])
    )
  )

(read-all)
