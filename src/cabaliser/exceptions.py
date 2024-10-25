'''
    Exception Objects
'''


class WidgetException(Exception):
    '''
        General widget exception object
    '''
    def __init__(self, msg=None, **kwargs):
        # Let the doc strings be the default error message
        if msg is None:
            msg = self.__doc__
        super().__init__(msg, **kwargs)


class WidgetNotDecomposedException(WidgetException):
    '''
        Exception raised if a widget has not been
        decomposed
    '''


class WidgetDecomposedException(WidgetException):
    '''
        Exception raised if a widget has already
        been decomposed
    '''


class WidgetNotEnoughQubitsException(WidgetException):
    '''
        The widget does not contain enough qubits
        to support that operation
    '''


class WidgetQubitIndexException(WidgetException):
    '''
        The targeted index is greater than the
        maximum allowable index for this widget
    '''


class ScheduleException(WidgetException):
    """
        Schedule does not consume widget
    """
