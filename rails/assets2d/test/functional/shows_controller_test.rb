require File.dirname(__FILE__) + '/../test_helper'
require 'shows_controller'

# Re-raise errors caught by the controller.
class ShowsController; def rescue_action(e) raise e end; end

class ShowsControllerTest < Test::Unit::TestCase
  fixtures :shows

  def setup
    @controller = ShowsController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = shows(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:shows)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:show)
    assert assigns(:show).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:show)
  end

  def test_create
    num_shows = Show.count

    post :create, :show => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_shows + 1, Show.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:show)
    assert assigns(:show).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Show.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Show.find(@first_id)
    }
  end
end
