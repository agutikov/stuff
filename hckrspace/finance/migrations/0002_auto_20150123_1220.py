# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime


class Migration(migrations.Migration):

    dependencies = [
        ('finance', '0001_initial'),
    ]

    operations = [
        migrations.AddField(
            model_name='total',
            name='direction',
            field=models.BooleanField(default=None),
            preserve_default=True,
        ),
        migrations.AlterField(
            model_name='transaction',
            name='timestamp',
            field=models.DateTimeField(blank=True, default=datetime.datetime(2015, 1, 23, 12, 20, 43, 127485)),
        ),
    ]
