#lang racket
(require "helpers.rkt")
(provide read-input 
         valid-input?
         invalid-in-msg)

(define VALID-INPUTS (list "find"))

(define (read-input)
  (read-line)
  )

(define (valid-input? s)
  [and (not (null? s))
       (string? s)
       (member (car (string-split s " ")) VALID-INPUTS)]
  )

(define (invalid-in-msg)
  (show "Invalid input. Try again.")
  )

