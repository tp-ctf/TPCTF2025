from django.shortcuts import render
from django.db import connection

# Create your views here.
from django.http import HttpResponse,HttpRequest
from .models import AdminUser,Blog
import os

def index(request:HttpRequest):
    return HttpResponse('Welcome to TPCTF 2025')

def flag(request:HttpRequest):
    if request.method != 'POST':
        return HttpResponse('Welcome to TPCTF 2025')
    username = request.POST.get('username')
    if username != 'admin':
        return HttpResponse('you are not admin.')
    password = request.POST.get('password')
    users:AdminUser = AdminUser.objects.raw("SELECT * FROM blog_adminuser WHERE username='%s' and password ='%s'" % (username,password))
    try:
        assert password == users[0].password
        return HttpResponse(os.environ.get('FLAG'))
    except:
        return HttpResponse('wrong password')
