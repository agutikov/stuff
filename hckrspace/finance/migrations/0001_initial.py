# -*- coding: utf-8 -*-
from __future__ import unicode_literals

from django.db import models, migrations
import datetime


class Migration(migrations.Migration):

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Balance',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('f_value', models.IntegerField()),
            ],
            options={
                'verbose_name': 'balance',
                'verbose_name_plural': 'balances',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Bill',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('details', models.TextField(null=True, blank=True)),
                ('f_value', models.IntegerField()),
                ('from_date', models.DateField(null=True, blank=True)),
                ('till_date', models.DateField(null=True, blank=True)),
                ('due_date', models.DateField()),
            ],
            options={
                'verbose_name': 'bill',
                'verbose_name_plural': 'bills',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Contract',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('title', models.CharField(max_length=200)),
                ('details', models.TextField(null=True, blank=True)),
                ('f_value', models.IntegerField(null=True, blank=True)),
            ],
            options={
                'verbose_name': 'contract',
                'verbose_name_plural': 'contracts',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Currency',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('name', models.CharField(unique=True, max_length=100)),
                ('code_num_3', models.CharField(unique=True, max_length=3)),
                ('code_ascii_3', models.CharField(unique=True, max_length=3)),
                ('exponent', models.IntegerField()),
            ],
            options={
                'verbose_name': 'currency',
                'verbose_name_plural': 'currency',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Event',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('f_value', models.IntegerField()),
                ('currency', models.ForeignKey(to='finance.Currency')),
            ],
            options={
                'verbose_name': 'event',
                'verbose_name_plural': 'events',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Membership_fee',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('comment', models.TextField(null=True, blank=True)),
            ],
            options={
                'verbose_name': 'membership_fee',
                'verbose_name_plural': 'membership_fees',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Payment',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('comment', models.TextField(null=True, blank=True)),
                ('bill', models.ForeignKey(to='finance.Bill', related_name='payment', unique=True)),
            ],
            options={
                'verbose_name': 'payment',
                'verbose_name_plural': 'payments',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Subject',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('extern_id', models.CharField(max_length=256)),
                ('extern_type', models.IntegerField()),
            ],
            options={
                'verbose_name': 'subject (user)',
                'verbose_name_plural': 'subjects (users)',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Total',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('f_value', models.IntegerField()),
                ('currency', models.ForeignKey(to='finance.Currency')),
            ],
            options={
                'verbose_name': 'total',
                'verbose_name_plural': 'totals',
            },
            bases=(models.Model,),
        ),
        migrations.CreateModel(
            name='Transaction',
            fields=[
                ('id', models.AutoField(auto_created=True, serialize=False, verbose_name='ID', primary_key=True)),
                ('timestamp', models.DateTimeField(blank=True, default=datetime.datetime(2015, 1, 21, 16, 20, 57, 997240))),
                ('description', models.TextField(null=True, blank=True)),
            ],
            options={
                'verbose_name': 'transaction',
                'verbose_name_plural': 'transactions',
            },
            bases=(models.Model,),
        ),
        migrations.AddField(
            model_name='total',
            name='transaction',
            field=models.ForeignKey(to='finance.Transaction', related_name='totals'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='payment',
            name='transaction',
            field=models.ForeignKey(to='finance.Transaction', related_name='payment', unique=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='membership_fee',
            name='subject',
            field=models.ForeignKey(to='finance.Subject', related_name='membership_fees'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='membership_fee',
            name='transaction',
            field=models.ForeignKey(to='finance.Transaction', related_name='membership_fee', unique=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='event',
            name='transaction',
            field=models.ForeignKey(to='finance.Transaction', related_name='events'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='contract',
            name='currency',
            field=models.ForeignKey(to='finance.Currency', null=True, blank=True),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='bill',
            name='currency',
            field=models.ForeignKey(to='finance.Currency'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='bill',
            name='destination',
            field=models.ForeignKey(to='finance.Contract', related_name='bills'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='balance',
            name='currency',
            field=models.ForeignKey(to='finance.Currency'),
            preserve_default=True,
        ),
        migrations.AddField(
            model_name='balance',
            name='transaction',
            field=models.ForeignKey(to='finance.Transaction', related_name='balances'),
            preserve_default=True,
        ),
    ]
