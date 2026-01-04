# A Lisp interpreter written in the C programming language

## Setup
- Linux: 
    - `sudo apt-get install build-essential`
    - `sudo apt-get install libreadline-dev`
- Mac: 
    - install command line tools 
    - `xcode-select --install`
    - `brew install readline`

## Compile and run
    -  `gcc -std=c99 -Wall -g lisp.c -lreadline -o lisp -lm`
    -  `./lisp`
    -  `valgrind --leak-check=full ./lisp`

## Done
- Try simple arithmetic operators +, -, *, /, %, ^, min, max 
- `(+ 1 (* 2 3))`
- `(* 1 (* 2 3) (- 5 4) (/ 6 3))`
- `(^ 4 2)`
- `(% 6 4)`
- `(min 8 4 (max (+ 5 9) 3 2 6))`

## TODO 
- My aim is for the Lisp interpreter to be able to run this code from Paul Graham's version of the Lisp 1.5

```clojure
; The Lisp defined in McCarthy's 1960 paper, translated into CL.
; Assumes only quote, atom, eq, cons, car, cdr, cond.
; Bug reports to lispcode@paulgraham.com.

(defun null. (x)       
  (eq x '()))

(defun and. (x y)
  (cond (x (cond (y 't) ('t '())))
        ('t '())))

(defun not. (x)
  (cond (x '())
        ('t 't)))

(defun append. (x y)
  (cond ((null. x) y)
        ('t (cons (car x) (append. (cdr x) y)))))

(defun list. (x y)
  (cons x (cons y '())))

(defun pair. (x y)
  (cond ((and. (null. x) (null. y)) '())
        ((and. (not. (atom x)) (not. (atom y)))
         (cons (list. (car x) (car y))
               (pair. (cdr x) (cdr y))))))

(defun assoc. (x y)
  (cond ((eq (caar y) x) (cadar y))
        ('t (assoc. x (cdr y)))))

(defun eval. (e a)
  (cond
    ((atom e) (assoc. e a))
    ((atom (car e))
     (cond
       ((eq (car e) 'quote) (cadr e))
       ((eq (car e) 'atom)  (atom   (eval. (cadr e) a)))
       ((eq (car e) 'eq)    (eq     (eval. (cadr e) a)
                                    (eval. (caddr e) a)))
       ((eq (car e) 'car)   (car    (eval. (cadr e) a)))
       ((eq (car e) 'cdr)   (cdr    (eval. (cadr e) a)))
       ((eq (car e) 'cons)  (cons   (eval. (cadr e) a)
                                    (eval. (caddr e) a)))
       ((eq (car e) 'cond)  (evcon. (cdr e) a))
       ('t (eval. (cons (assoc. (car e) a)
                        (cdr e))
                  a))))
    ((eq (caar e) 'label)
     (eval. (cons (caddar e) (cdr e))
            (cons (list. (cadar e) (car e)) a)))
    ((eq (caar e) 'lambda)
     (eval. (caddar e)
            (append. (pair. (cadar e) (evlis. (cdr e) a))
                     a)))))

(defun evcon. (c a)
  (cond ((eval. (caar c) a)
         (eval. (cadar c) a))
        ('t (evcon. (cdr c) a))))

(defun evlis. (m a)
  (cond ((null. m) '())
        ('t (cons (eval.  (car m) a)
                  (evlis. (cdr m) a)))))


```
## Reference
- https://en.cppreference.com/w/c.html
- https://www.buildyourownlisp.com/
