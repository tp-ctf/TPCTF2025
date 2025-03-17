from django.db import models
from django.db import connection

class AdminUser(models.Model):
    username = models.CharField(max_length=20)
    password = models.CharField(max_length=20)

class Blog(models.Model):
    id = models.IntegerField(primary_key=True)
    title = models.CharField(max_length=20)
    content = models.TextField()



