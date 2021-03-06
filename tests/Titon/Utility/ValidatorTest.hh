<?hh
namespace Titon\Utility;

use Titon\Test\TestCase;

/**
 * @property \Titon\Utility\Validator $object
 */
class ValidatorTest extends TestCase {

    protected function setUp() {
        parent::setUp();

        $this->object = new Validator(Map {
            'username' => 'miles',
            'email' => 'miles@titon' // invalid
        });
    }

    public function testAddGetErrors() {
        $this->assertEquals(Map {}, $this->object->getErrors());

        $this->object->addError('username', 'Invalid username');
        $this->assertEquals(Map {'username' => 'Invalid username'}, $this->object->getErrors());
    }

    public function testAddGetFields() {
        $this->assertEquals(Map {}, $this->object->getFields());

        $this->object->addField('username', 'Username');
        $this->assertEquals(Map {'username' => 'Username'}, $this->object->getFields());
    }

    public function testAddGetRules() {
        $this->assertEquals(Map {}, $this->object->getRules());

        // via addRule()
        $this->object
            ->addField('basicRule', 'Basic Rule')
                ->addRule('basicRule', 'alphaNumeric', 'Custom alpha-numeric message')
                ->addRule('basicRule', 'between', 'May only be between 0 and 100 characters', Vector {0, 100}); // use default message

        $this->assertEquals(Map {
            'basicRule' => Map {
                'alphaNumeric' => shape(
                    'rule' => 'alphaNumeric',
                    'message' => 'Custom alpha-numeric message',
                    'options' => Vector {}
                ),
                'between' => shape(
                    'rule' => 'between',
                    'message' => 'May only be between 0 and 100 characters',
                    'options' => Vector {0, 100}
                )
            }
        }, $this->object->getRules());

        // via third argument in addField()
        $this->object->addMessages(Map {
            'phone' => 'Invalid phone number',
            'email' => 'Please provide an email',
            'ext' => 'Valid extensions are {0}',
            'ip' => 'Please provide an IPv4'
        });

        $this->object->addField('advRule', 'Advanced Rule', Map {
            'phone' => Vector {},
            'email' => Vector {},
            'ext' => Vector {Vector {'txt', 'pdf'}},
            'ip' => Vector {Validate::IPV4}
        });

        $this->assertEquals(Map {
            'basicRule' => Map {
                'alphaNumeric' => shape(
                    'rule' => 'alphaNumeric',
                    'message' => 'Custom alpha-numeric message',
                    'options' => Vector {}
                ),
                'between' => shape(
                    'rule' => 'between',
                    'message' => 'May only be between 0 and 100 characters',
                    'options' => Vector {0, 100}
                )
            },
            'advRule' => Map {
                'phone' => shape(
                    'rule' => 'phone',
                    'message' => 'Invalid phone number',
                    'options' => Vector {}
                ),
                'email' => shape(
                    'rule' => 'email',
                    'message' => 'Please provide an email',
                    'options' => Vector {}
                ),
                'ext' => shape(
                    'rule' => 'ext',
                    'message' => 'Valid extensions are {0}',
                    'options' => Vector {Vector {'txt', 'pdf'}}
                ),
                'ip' => shape(
                    'rule' => 'ip',
                    'message' => 'Please provide an IPv4',
                    'options' => Vector {Validate::IPV4}
                )
            }
        }, $this->object->getRules());
    }

    /**
     * @expectedException \Titon\Utility\Exception\InvalidArgumentException
     */
    public function testAddRuleInvalidField() {
        $this->object->addRule('field', 'rule', 'message');
    }

    public function testMessages() {
        $this->object
            ->addField('username', 'Username')
                ->addRule('username', 'between', 'May only be between {0} and {1} characters', Vector {25, 45})
            ->addField('email', 'Email')
                ->addRule('email', 'ext', 'May only have extension: {0}', Vector {Vector {'gif', 'png'}});

        $this->assertEquals(Map {
            'between' => 'May only be between {0} and {1} characters',
            'ext' => 'May only have extension: {0}'
        }, $this->object->getMessages());

        $this->object->validate();

        $this->assertEquals(Map {
            'username' => 'May only be between 25 and 45 characters',
            'email' => 'May only have extension: gif, png'
        }, $this->object->getErrors());
    }

    public function testMessageDetection() {
        $this->object
            ->addField('username', 'Username')
                ->addRule('username', 'notEmpty', '')
                ->addRule('username', 'between', '{field} may only be between {0} and {1} characters', Vector {10, 20})
                ->addRule('username', 'alpha', '')
            ->addMessages(Map {
                'notEmpty' => '{field} cannot be empty'
            });

        $this->object->addMessages(Map {
            'alpha' => '{title} may only be alpha'
        });

        // {field} insertion
        $this->object->validate();

        $this->assertEquals(Map {
            'username' => 'username may only be between 10 and 20 characters'
        }, $this->object->getErrors());

        // {title} insertion
        $this->object->setData(Map {'username' => 'foo long name bar!'});
        $this->object->validate();

        $this->assertEquals(Map {
            'username' => 'Username may only be alpha'
        }, $this->object->getErrors());
    }

    /**
     * @expectedException \Titon\Utility\Exception\InvalidValidationRuleException
     */
    public function testMessagesErrorOnMissing() {
        $this->object
            ->addField('username', 'Username')
                ->addRule('username', 'notEmpty', '');

        $this->object->validate();
    }

    public function testReset() {
        $this->assertEquals(Map {'username' => 'miles', 'email' => 'miles@titon'}, $this->object->getData());
        $this->object->reset();
        $this->assertEquals(Map {}, $this->object->getData());
    }

    public function testSetGetData() {
        $this->object->setData(Map {'foo' => 'bar'});
        $this->assertEquals(Map {'foo' => 'bar'}, $this->object->getData());
    }

    public function testSplitShorthand() {
        // only a rule
        $this->assertEquals(shape(
            'rule' => 'boolean',
            'message' => '',
            'options' => Vector {}
        ), $this->object->splitShorthand('boolean'));

        // rule with 1 param
        $this->assertEquals(shape(
            'rule' => 'decimal',
            'message' => '',
            'options' => Vector {1}
        ), $this->object->splitShorthand('decimal:1'));

        // rule with 2 params
        $this->assertEquals(shape(
            'rule' => 'between',
            'message' => '',
            'options' => Vector {1, 10}
        ), $this->object->splitShorthand('between:1,10'));

        // only a rule and message
        $this->assertEquals(shape(
            'rule' => 'boolean',
            'message' => 'Must be a boolean!',
            'options' => Vector {}
        ), $this->object->splitShorthand('boolean::Must be a boolean!'));

        // rule with 2 params and message
        $this->assertEquals(shape(
            'rule' => 'between',
            'message' => 'Must be between 1:10!',
            'options' => Vector {1, 10}
        ), $this->object->splitShorthand('between:1,10:Must be between 1:10!'));
    }

    public function testValidate() {
        $this->object->addField('username', 'Username')->addRule('username', 'alpha', 'Not alpha');

        $this->assertTrue($this->object->validate());
        $this->assertEquals(Map {}, $this->object->getErrors());

        // this will fail
        $this->object->addField('email', 'Email')->addRule('email', 'email', 'Invalid email');

        $this->assertFalse($this->object->validate());
        $this->assertEquals(Map {'email' => 'Invalid email'}, $this->object->getErrors());
    }

    public function testValidateFailsNoData() {
        $this->object->reset();

        $this->assertFalse($this->object->validate());
    }

    /**
     * @expectedException \Titon\Utility\Exception\InvalidValidationRuleException
     */
    public function testValidateMissingRule() {
        $this->object
            ->addField('username', 'Username')
                ->addRule('username', 'fooBar', 'A message');

        $this->object->validate();
    }

    public function testMakeFromShorthand() {
        // simple rule
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => 'alphaNumeric',
            'field2' => 123 // ignored
        });

        $this->assertEquals(Map {
            'field' => Map {
                'alphaNumeric' => shape(
                    'rule' => 'alphaNumeric',
                    'message' => '',
                    'options' => Vector {}
                )
            }
        }, $obj->getRules());

        // simple 2 rules
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => 'alphaNumeric|boolean'
        });

        $this->assertEquals(Map {
            'field' => Map {
                'alphaNumeric' => shape(
                    'rule' => 'alphaNumeric',
                    'message' => '',
                    'options' => Vector {}
                ),
                'boolean' => shape(
                    'rule' => 'boolean',
                    'message' => '',
                    'options' => Vector {}
                )
            }
        }, $obj->getRules());

        // simple 2 rules with options
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => 'between:5,10|equal:7'
        });

        $this->assertEquals(Map {
            'field' => Map {
                'between' => shape(
                    'rule' => 'between',
                    'message' => '',
                    'options' => Vector {5, 10}
                ),
                'equal' => shape(
                    'rule' => 'equal',
                    'message' => '',
                    'options' => Vector {7}
                )
            }
        }, $obj->getRules());

        // split 2 rules
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => Vector {'between:5,10', 'equal:7'}
        });

        $this->assertEquals(Map {
            'field' => Map {
                'between' => shape(
                    'rule' => 'between',
                    'message' => '',
                    'options' => Vector {5, 10}
                ),
                'equal' => shape(
                    'rule' => 'equal',
                    'message' => '',
                    'options' => Vector {7}
                )
            }
        }, $obj->getRules());

        // nested 2 rules
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => Map {
                'title' => 'Field',
                'rules' => 'between:5,10|equal:7'
            }
        });

        $this->assertEquals(Map {
            'field' => Map {
                'between' => shape(
                    'rule' => 'between',
                    'message' => '',
                    'options' => Vector {5, 10}
                ),
                'equal' => shape(
                    'rule' => 'equal',
                    'message' => '',
                    'options' => Vector {7}
                )
            }
        }, $obj->getRules());

        // nested split 2 rules
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => Map {
                'title' => 'Field',
                'rules' => Vector {'between:5,10', 'equal:7'}
            }
        });

        $this->assertEquals(Map {
            'field' => Map {
                'between' => shape(
                    'rule' => 'between',
                    'message' => '',
                    'options' => Vector {5, 10}
                ),
                'equal' => shape(
                    'rule' => 'equal',
                    'message' => '',
                    'options' => Vector {7}
                )
            }
        }, $obj->getRules());

        // advanced multiple rules
        $obj = Validator::makeFromShorthand(Map {}, Map {
            'field' => Map {
                'rules' => Vector {
                    'phone::Invalid phone number',
                    'email::Please provide an email',
                    'ext:txt:Valid extensions are txt, pdf',
                    'ip:' . Validate::IPV4 . ':Please provide an IPv4'
                }
            }
        });

        $this->assertEquals(Map {
            'field' => Map {
                'phone' => shape(
                    'rule' => 'phone',
                    'message' => 'Invalid phone number',
                    'options' => Vector {}
                ),
                'email' => shape(
                    'rule' => 'email',
                    'message' => 'Please provide an email',
                    'options' => Vector {}
                ),
                'ext' => shape(
                    'rule' => 'ext',
                    'message' => 'Valid extensions are txt, pdf',
                    'options' => Vector {'txt'}
                ),
                'ip' => shape(
                    'rule' => 'ip',
                    'message' => 'Please provide an IPv4',
                    'options' => Vector {Validate::IPV4}
                )
            }
        }, $obj->getRules());
    }

}