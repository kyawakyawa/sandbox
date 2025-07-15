# Hydra-test

```bash
$ python my_app.py 
db:
  driver: mysql
  pass: secret
  user: omry
params:
  hoge: hoge
``` 

```bash
$ python my_app.py +experiment=test
db:
  driver: postgresql
  pass: drowssap
  timeout: 20
  user: postgres_user
params:
  hoge: fuga

```
